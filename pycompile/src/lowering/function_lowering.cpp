//
// Created by matthew on 3/24/26.
//

#include "function_lowering.h"


mlir::LogicalResult CallLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                  mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // operands[0] = callee, operands[1..] = args (after type conversion)
    const mlir::Value callee = operands[0];
    const mlir::ArrayRef<mlir::Value> args = operands.drop_front(1);
    const int64_t argc = static_cast<int64_t>(args.size());

    // declare: extern PyObj* pyir_call(PyObj* callee, Value** args, int64_t argc)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx), i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_call", fnType);

    // Allocate args array on the stack: PyObj*[argc]
    mlir::Value argsPtr;
    if (argc > 0) {
        const mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), argc);
        mlir::LLVM::AllocaOp alloca = mlir::LLVM::AllocaOp::create(
                rewriter, loc, ptrType(ctx), arrType,
                mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(1)));

        // Store each arg into the array
        for (int64_t i = 0; i < argc; i++) {
            mlir::LLVM::ConstantOp idx =
                    mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));
            mlir::LLVM::GEPOp gep =
                    mlir::LLVM::GEPOp::create(rewriter, loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});
            mlir::LLVM::StoreOp::create(rewriter, loc, args[i], gep);
        }
        argsPtr = alloca;
    } else
        // nullptr for empty args
        argsPtr = mlir::LLVM::ZeroOp::create(rewriter, loc, ptrType(ctx));

    mlir::LLVM::ConstantOp argcVal =
            mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(argc));

    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{callee, argsPtr, argcVal});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult PushScopeLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_pushScope", op, operands, rewriter, false);
}


mlir::LogicalResult PopScopeLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_popScope", op, operands, rewriter, false);
}


mlir::LogicalResult MakeFunctionLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                          mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    pyir::MakeFunction makeFunc = mlir::cast<pyir::MakeFunction>(op);
    const std::string fnName = makeFunc.getFnName().str();

    // declare: extern PyObj* pyir_makeFunction(char* display_name, void* fn_ptr)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp runtimeFn = getOrInsertRuntimeFn(rewriter, module, "pyir_makeFunction", fnType);

    // Get function pointer from symbol table
    const mlir::Value fnPtr = mlir::LLVM::AddressOfOp::create(rewriter, loc, ptrType(ctx), fnName);
    const std::string globalName = "__pyir_str_fn_" + makeFunc.getFnName().str();
    const mlir::Value namePtr = getOrInsertStringConstant(rewriter, module, loc, globalName, makeFunc.getFnName());

    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, runtimeFn, mlir::ValueRange{namePtr, fnPtr});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult ReturnValueLowering::matchAndRewrite(mlir::Operation* op,
                                                         const mlir::ArrayRef<mlir::Value> operands,
                                                         mlir::ConversionPatternRewriter& rewriter) const {
    rewriter.replaceOpWithNewOp<mlir::LLVM::ReturnOp>(op, operands[0]);
    return mlir::success();
}
