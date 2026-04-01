//
// Created by matthew on 3/24/26.
//

#include "lowering/function_lowering.h"


mlir::LogicalResult CallLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                  mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // operands[0] = callee, operands[1..] = args (after type conversion)
    const mlir::Value callee = operands[0];
    const mlir::ArrayRef<mlir::Value> args = operands.drop_front(1);
    const int64_t argc = static_cast<int64_t>(args.size());

    // declare: extern Value* pyir_call(Value* callee, Value** args, int64_t argc)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx), i64Type(ctx)});
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
            mlir::LLVM::ConstantOp idx =
                    rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));
            mlir::LLVM::GEPOp gep =
                    rewriter.create<mlir::LLVM::GEPOp>(loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});
            rewriter.create<mlir::LLVM::StoreOp>(loc, args[i], gep);
        }
        argsPtr = alloca;
    } else
        // null pointer for empty args
        argsPtr = rewriter.create<mlir::LLVM::ZeroOp>(loc, ptrType(ctx));

    mlir::LLVM::ConstantOp argcVal =
            rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(argc));

    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{callee, argsPtr, argcVal});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult PushScopeLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_pushScope()
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_pushScope", fnType);

    rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult PopScopeLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_popScope()
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_popScope", fnType);

    rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult MakeFunctionLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                          mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    pyir::MakeFunction makeFunc = mlir::cast<pyir::MakeFunction>(op);
    const std::string fnName = makeFunc.getFnName().str();

    // declare: extern Value* pyir_makeFunction(char* display_name, void* fn_ptr)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp runtimeFn = getOrInsertRuntimeFn(rewriter, module, "pyir_makeFunction", fnType);

    // Get function pointer from symbol table
    const mlir::Value fnPtr = rewriter.create<mlir::LLVM::AddressOfOp>(loc, ptrType(ctx), fnName);
    const std::string globalName = "__pyir_str_fn_" + makeFunc.getFnName().str();
    const mlir::Value namePtr = getOrInsertStringConstant(rewriter, module, loc, globalName, makeFunc.getFnName());

    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, runtimeFn, mlir::ValueRange{namePtr, fnPtr});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult ReturnValueLowering::matchAndRewrite(mlir::Operation* op,
                                                         const mlir::ArrayRef<mlir::Value> operands,
                                                         mlir::ConversionPatternRewriter& rewriter) const {
    rewriter.replaceOpWithNewOp<mlir::LLVM::ReturnOp>(op, operands[0]);
    return mlir::success();
}
