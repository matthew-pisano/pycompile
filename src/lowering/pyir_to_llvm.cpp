//
// Created by matthew on 3/9/26.
//

#include "lowering/pyir_to_llvm.h"

#include "pyir/pyir_ops.h"
#include "pyir/pyir_types.h"

#include <mlir/Conversion/LLVMCommon/ConversionTarget.h>
#include <mlir/Conversion/LLVMCommon/TypeConverter.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Transforms/DialectConversion.h>
#include <mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h>
#include <mlir/Transforms/Passes.h>


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
 * Returns an opaque LLVM pointer type (!llvm.ptr) in the given context.
 * Used as the LLVM representation of pyir::Value* throughout the lowering.
 */
static mlir::LLVM::LLVMPointerType ptrType(mlir::MLIRContext* ctx) {
    return mlir::LLVM::LLVMPointerType::get(ctx);
}


/**
 * Returns a 64-bit integer type in the given context.
 * Used for argument counts and integer constants.
 */
static mlir::Type i64Type(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 64);
}


/**
 * Returns an 8-bit integer type in the given context.
 * Used as the element type for string constant arrays.
 */
static mlir::Type i8Type(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 8);
}


/**
 * Looks up an external runtime function by name in the module, inserting a declaration if one does not already exist.
 *
 * This is used to lazily declare runtime functions (e.g. pyir_load_name, pyir_call) only when an op that needs them is
 * encountered during lowering, avoiding unused declarations in the output.
 *
 * @param rewriter The pattern rewriter, used to insert the declaration.
 * @param module The module to insert the declaration into.
 * @param name The symbol name of the runtime function.
 * @param fnType The LLVM function type of the declaration.
 * @return The existing or newly created LLVMFuncOp declaration.
 */
static mlir::LLVM::LLVMFuncOp getOrInsertRuntimeFn(mlir::PatternRewriter& rewriter, mlir::ModuleOp module,
                                                   llvm::StringRef name, mlir::LLVM::LLVMFunctionType fnType) {
    if (const mlir::LLVM::LLVMFuncOp fn = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>(name))
        return fn;

    mlir::PatternRewriter::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(module.getBody());
    return rewriter.create<mlir::LLVM::LLVMFuncOp>(rewriter.getUnknownLoc(), name, fnType,
                                                   mlir::LLVM::Linkage::External);
}


/**
 * Creates a private global string constant in the module if one with the given name does not already exist, and
 * returns a pointer to it.
 *
 * The string is stored as a null-terminated i8 array in the module's rodata section. Identical strings are
 * deduplicated by name.
 *
 * @param rewriter The pattern rewriter, used to insert the global.
 * @param module The module to insert the global into.
 * @param loc The location to attach to the global op.
 * @param name The symbol name for the global.
 * @param str The string value to store, without null terminator.
 * @return An !llvm.ptr value pointing to the global.
 */
static mlir::Value getOrInsertStringConstant(mlir::ConversionPatternRewriter& rewriter, mlir::ModuleOp module,
                                             const mlir::Location loc, llvm::StringRef name,
                                             const llvm::StringRef str) {
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


/**
 * Base class for all PyIR to LLVM conversion patterns.
 *
 * Provides a convenience method for retrieving the parent ModuleOp, which is needed by patterns that insert global
 * declarations or runtime function declarations.
 */
struct PyIROpConversion : mlir::ConversionPattern {
    PyIROpConversion(llvm::StringRef opName, const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        ConversionPattern(tc, opName, /*benefit=*/1, ctx) {
    }

    static mlir::ModuleOp getModule(mlir::Operation* op) {
        return op->getParentOfType<mlir::ModuleOp>();
    }
};


/**
 * Lowers pyir.load_name to a call to the runtime function pyir_load_name.
 *
 * The name string is stored as a global constant and passed as a const char* pointer. The runtime resolves the name
 * against the builtin table and returns a heap-allocated Value*.
 *
 * pyir.load_name "print"
 *     %ptr = llvm.mlir.addressof @__pyir_str_print
 *     %val = llvm.call @pyir_load_name(%ptr)
 */
struct LoadNameLowering : PyIROpConversion {
    LoadNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadName::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::LoadName loadName = mlir::cast<pyir::LoadName>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern Value* pyir_load_name(const char* name)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_name", fnType);

        // create a global string constant for the name
        const std::string globalName = "__pyir_str_" + loadName.getVarName().str();

        // get pointer to the global string
        const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, loadName.getVarName());

        // call pyir_load_name
        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});

        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.load_const to a call to the appropriate runtime constant constructor, depending on the attribute type:
 *
 *   StringAttr -> pyir_load_const_str(const char*)
 *   IntegerAttr -> pyir_load_const_int(int64_t)
 *
 * String constants are stored as global i8 arrays. Integer constants are passed directly as i64 values. Both return a
 * heap-allocated Value*.
 */
struct LoadConstLowering : PyIROpConversion {
    LoadConstLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadConst::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::LoadConst loadConst = mlir::cast<pyir::LoadConst>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        mlir::Value result;

        if (const mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue())) {
            // declare: extern Value* pyir_load_const_str(const char* str)
            const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
            mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_const_str", fnType);

            // create global string constant
            const std::string str = strAttr.getValue().str();
            const std::string gName = "__pyir_const_str_" + std::to_string(std::hash<std::string>{}(str));
            const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, gName, str);
            mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});
            result = call.getResult();

        } else if (const mlir::IntegerAttr intAttr = mlir::dyn_cast<mlir::IntegerAttr>(loadConst.getValue())) {
            // declare: extern Value* pyir_load_const_int(int64_t val)
            const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {i64Type(ctx)});
            mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_const_int", fnType);
            mlir::LLVM::ConstantOp intVal = rewriter.create<mlir::LLVM::ConstantOp>(
                    loc, i64Type(ctx), rewriter.getI64IntegerAttr(intAttr.getInt()));
            mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{intVal});
            result = call.getResult();
        } else
            return mlir::failure(); // unhandled constant type

        rewriter.replaceOp(op, result);
        return mlir::success();
    }
};


/**
 * Lowers pyir.push_null to a call to pyir_push_null().
 *
 * push_null is a CPython calling convention artifact that places a null sentinel below the callee on the stack. In the
 * lowered IR it becomes a runtime call that returns a null Value* sentinel, which the Call lowering discards.
 */
struct PushNullLowering : PyIROpConversion {
    PushNullLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PushNull::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_push_null", fnType);
        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{});
        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.call to a call to the runtime function pyir_call.
 *
 * Arguments are stack-allocated as a Value*[] array and passed by pointer along with the argument count. The callee is
 * passed as a Value* pointer.
 *
 * pyir.call %callee(%arg0, %arg1)
 *     %arr    = llvm.alloca [2 x !llvm.ptr]
 *     %gep0   = llvm.gep %arr[0]
 *               llvm.store %arg0, %gep0
 *     %gep1   = llvm.gep %arr[1]
 *               llvm.store %arg1, %gep1
 *     %result = llvm.call @pyir_call(%callee, %arr, 2)
 *
 * For zero-argument calls, a null pointer is passed instead of an array.
 */
struct CallLowering : PyIROpConversion {
    CallLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::Call::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // operands[0] = callee, operands[1..] = args (after type conversion)
        const mlir::Value callee = operands[0];
        const mlir::ArrayRef<mlir::Value> args = operands.drop_front(1);
        const int64_t argc = static_cast<int64_t>(args.size());

        // declare: extern Value* pyir_call(Value* callee, Value** args, int64_t argc)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                ptrType(ctx), {ptrType(ctx), ptrType(ctx), i64Type(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_call", fnType);

        // allocate args array on the stack: Value*[argc]
        mlir::Value argsPtr;
        if (argc > 0) {
            mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), argc);
            mlir::LLVM::AllocaOp alloca = rewriter.create<mlir::LLVM::AllocaOp>(
                    loc, ptrType(ctx), arrType,
                    rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(1)));

            // store each arg into the array
            for (int64_t i = 0; i < argc; i++) {
                mlir::LLVM::ConstantOp idx = rewriter.create<mlir::LLVM::ConstantOp>(
                        loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));
                mlir::LLVM::GEPOp gep = rewriter.create<mlir::LLVM::GEPOp>(
                        loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});
                rewriter.create<mlir::LLVM::StoreOp>(loc, args[i], gep);
            }
            argsPtr = alloca;
        } else
        // null pointer for empty args
            argsPtr = rewriter.create<mlir::LLVM::ZeroOp>(loc, ptrType(ctx));

        mlir::LLVM::ConstantOp argcVal = rewriter.create<mlir::LLVM::ConstantOp>(
                loc, i64Type(ctx), rewriter.getI64IntegerAttr(argc));

        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(
                loc, fn, mlir::ValueRange{callee, argsPtr, argcVal});

        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.pop_top by erasing it.
 *
 * pop_top discards the top of the Python evaluation stack. In SSA form the value simply has no uses, so the op itself
 * can be erased. DCE will subsequently remove the producing op if it is Pure and has no other uses.
 */
struct PopTopLowering : PyIROpConversion {
    PopTopLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PopTop::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        rewriter.eraseOp(op);
        return mlir::success();
    }
};


/**
 * MLIR pass that lowers the entire PyIR dialect to the LLVM dialect.
 *
 * Applies all PyIR to LLVM conversion patterns along with the standard func to LLVM patterns. Marks the PyIR dialect
 * as illegal and the LLVM dialect as legal, so applyFullConversion will fail if any PyIR ops remain after the patterns
 * have been applied.
 */
struct PyIRToLLVMPass : mlir::PassWrapper<PyIRToLLVMPass, mlir::OperationPass<mlir::ModuleOp> > {
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

        mlir::LLVMConversionTarget target(*ctx);
        target.addIllegalDialect<pyir::PyIRDialect>();
        target.addLegalDialect<mlir::LLVM::LLVMDialect>();
        target.addLegalOp<mlir::ModuleOp>();

        if (mlir::failed(mlir::applyFullConversion(module, target, std::move(patterns))))
            signalPassFailure();
    }
};


void populatePyIRToLLVMPatterns(mlir::RewritePatternSet& patterns,
                                mlir::LLVMTypeConverter& typeConverter) {
    mlir::MLIRContext* ctx = patterns.getContext();
    patterns.add<
        LoadNameLowering,
        LoadConstLowering,
        PushNullLowering,
        CallLowering,
        PopTopLowering
    >(typeConverter, ctx);
}


std::unique_ptr<mlir::Pass> createPyIRToLLVMPass() {
    return std::make_unique<PyIRToLLVMPass>();
}


void lowerToLLVM(mlir::MLIRContext& ctx, const mlir::ModuleOp module) {
    ctx.loadDialect<mlir::LLVM::LLVMDialect>();

    mlir::PassManager pm(&ctx);

    pm.addPass(mlir::createCanonicalizerPass());

    // lower PyIR to LLVM dialect
    pm.addPass(createPyIRToLLVMPass());

    const llvm::LogicalResult result = pm.run(module);
    if (mlir::failed(result))
        throw std::runtime_error("Failed to lower PYIR module");
}
