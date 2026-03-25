//
// Created by matthew on 3/24/26.
//


#include "lowering/pyir_conversion_utils.h"


mlir::LLVM::LLVMPointerType ptrType(mlir::MLIRContext* ctx) {
    return mlir::LLVM::LLVMPointerType::get(ctx);
}


mlir::Type boolType(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 8);
}


mlir::Type i64Type(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 64);
}


mlir::Type i8Type(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 8);
}


mlir::Type f64Type(mlir::MLIRContext* ctx) {
    return mlir::Float64Type::get(ctx);
}


mlir::LLVM::LLVMFuncOp getOrInsertRuntimeFn(mlir::PatternRewriter& rewriter, mlir::ModuleOp module,
                                            llvm::StringRef name, mlir::LLVM::LLVMFunctionType fnType) {
    if (const mlir::LLVM::LLVMFuncOp fn = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>(name))
        return fn;

    mlir::PatternRewriter::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(module.getBody());
    return rewriter.create<mlir::LLVM::LLVMFuncOp>(rewriter.getUnknownLoc(), name, fnType,
                                                   mlir::LLVM::Linkage::External);
}


mlir::Value getOrInsertStringConstant(mlir::ConversionPatternRewriter& rewriter, mlir::ModuleOp module,
                                      const mlir::Location loc, llvm::StringRef name, const llvm::StringRef str) {
    auto* ctx = module.getContext();

    if (!module.lookupSymbol(name)) {
        mlir::PatternRewriter::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(module.getBody());
        auto strType = mlir::LLVM::LLVMArrayType::get(i8Type(ctx), str.size() + 1);
        rewriter.create<mlir::LLVM::GlobalOp>(
                loc, strType, /*isConstant=*/true,
                mlir::LLVM::Linkage::Private, name,
                rewriter.getStringAttr(str.str() + '\0'));
    }

    return rewriter.create<mlir::LLVM::AddressOfOp>(loc, ptrType(ctx), name);
}


mlir::LogicalResult PyIROpConversion::linkOpToRuntimeFunc(const std::string& func, mlir::Operation* op,
                                                          const mlir::ArrayRef<mlir::Value> operands,
                                                          mlir::ConversionPatternRewriter& rewriter,
                                                          const size_t argc) {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    mlir::LLVM::LLVMFunctionType fnType;
    if (argc == 1)
        fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    else if (argc == 2)
        fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx)});
    else
        throw std::runtime_error("Unsupported number of args in runtime function link");

    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, func, fnType);

    mlir::LLVM::CallOp call;
    if (argc == 1)
        call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0]});
    else
        call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0], operands[1]});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}
