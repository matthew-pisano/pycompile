//
// Created by matthew on 3/9/26.
//

#include "lowering/pyir_lowering.h"

#include <filesystem>

#include "pyir/pyir_attrs.h"
#include "pyir/pyir_ops.h"
#include "pyir/pyir_types.h"

#include <mlir/Conversion/ArithToLLVM/ArithToLLVM.h>
#include <mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h>
#include <mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h>
#include <mlir/Conversion/LLVMCommon/ConversionTarget.h>
#include <mlir/Conversion/LLVMCommon/TypeConverter.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Transforms/DialectConversion.h>

#include <mlir/Transforms/Passes.h>

#include "lowering/builder_lowering.h"
#include "lowering/control_flow_lowering.h"
#include "lowering/function_lowering.h"
#include "lowering/logical_lowering.h"
#include "lowering/memory_lowering.h"
#include "lowering/pyir_conversion_utils.h"
#include "utils.h"


/**
 * Registers a conversion from pyir::ByteCodeObjectType to !llvm.ptr.
 *
 * All Python values are represented as opaque pointers to heap-allocated
 * pyir::Value objects in the runtime. This avoids ABI complexity around
 * passing std::variant by value across the LLVM boundary.
 * @param converter The LLVM type converter
 */
static void addPyIRTypeConversions(mlir::LLVMTypeConverter& converter) {
    converter.addConversion([](const pyir::ByteCodeObjectType type) -> mlir::Type {
        return mlir::LLVM::LLVMPointerType::get(type.getContext());
    });
}


/**
 * MLIR pass that lowers the entire PyIR dialect to the LLVM dialect.
 *
 * Applies all PyIR to LLVM conversion patterns along with the standard func to LLVM patterns. Marks the PyIR dialect
 * as illegal and the LLVM dialect as legal, so applyFullConversion will fail if any PyIR ops remain after the patterns
 * have been applied.
 */
struct PyIRToLLVMPass : mlir::PassWrapper<PyIRToLLVMPass, mlir::OperationPass<mlir::ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(PyIRToLLVMPass)

    void getDependentDialects(mlir::DialectRegistry& registry) const override {
        registry.insert<mlir::LLVM::LLVMDialect>();
    }

    void runOnOperation() override {
        const mlir::ModuleOp module = getOperation();
        mlir::MLIRContext* ctx = &getContext();

        mlir::LLVMTypeConverter typeConverter(ctx);
        addPyIRTypeConversions(typeConverter);

        mlir::RewritePatternSet patterns(ctx);
        populatePyIRToLLVMPatterns(patterns, typeConverter);
        mlir::populateFuncToLLVMConversionPatterns(typeConverter, patterns);
        mlir::arith::populateArithToLLVMConversionPatterns(typeConverter, patterns);
        mlir::cf::populateControlFlowToLLVMConversionPatterns(typeConverter, patterns);

        mlir::LLVMConversionTarget target(*ctx);
        target.addIllegalDialect<pyir::PyIRDialect>();
        target.addIllegalDialect<mlir::arith::ArithDialect>();
        target.addLegalDialect<mlir::LLVM::LLVMDialect>();
        target.addLegalOp<mlir::ModuleOp>();

        if (mlir::failed(mlir::applyFullConversion(module, target, std::move(patterns))))
            signalPassFailure();
    }
};


void populatePyIRToLLVMPatterns(mlir::RewritePatternSet& patterns, mlir::LLVMTypeConverter& typeConverter) {
    mlir::MLIRContext* ctx = patterns.getContext();
    patterns.add<IsTruthyLowering, ToBoolLowering, UnaryNegativeLowering, UnaryNotLowering, UnaryInvertLowering,
                 BinaryOpLowering, CompareOpLowering, LoadFastLowering, StoreFastLowering, LoadNameLowering,
                 StoreNameLowering, LoadConstLowering, PushNullLowering, CallLowering, PopTopLowering,
                 FormatSimpleLowering, BuildStringLowering, PushScopeLowering, PopScopeLowering, LoadArgLowering,
                 MakeFunctionLowering, ReturnValueLowering, BuildListLowering, ListExtendLowering>(typeConverter, ctx);
}


std::unique_ptr<mlir::Pass> createPyIRToLLVMPass() { return std::make_unique<PyIRToLLVMPass>(); }


void lowerToLLVMDialect(mlir::MLIRContext& ctx, const mlir::OwningOpRef<mlir::ModuleOp>& module) {
    ctx.loadDialect<mlir::LLVM::LLVMDialect>();

    mlir::PassManager pm(&ctx);

    pm.addPass(mlir::createCanonicalizerPass());
    // Lower PyIR to LLVM dialect
    pm.addPass(createPyIRToLLVMPass());

    mlir::Location errorLoc = mlir::UnknownLoc::get(&ctx);
    std::string errorMessage;
    mlir::ScopedDiagnosticHandler handler(&ctx, [&](const mlir::Diagnostic& diag) {
        if (diag.getSeverity() == mlir::DiagnosticSeverity::Error) {
            errorMessage = diag.str();
            errorLoc = diag.getLocation();
        }
        return mlir::success();
    });

    const llvm::LogicalResult result = pm.run(module.get());
    if (mlir::failed(result)) {
        size_t lineno = 0;
        size_t offset = 0;
        std::string moduleName = "<unknown>";
        if (const mlir::FileLineColLoc fileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(errorLoc)) {
            moduleName = std::filesystem::path(fileLoc.getFilename().str()).filename();
            lineno = fileLoc.getLine();
            offset = fileLoc.getColumn();
        }
        throw PyCompileError(errorMessage, moduleName, lineno, offset);
    }
}
