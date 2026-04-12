//
// Created by matthew on 3/24/26.
//

#include "lowering/control_flow_lowering.h"


mlir::LogicalResult PushNullLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    rewriter.replaceOpWithNewOp<mlir::LLVM::ZeroOp>(op, ptrType(op->getContext()));
    return mlir::success();
}


mlir::LogicalResult PopTopLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                    mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_decref", fnType);

    rewriter.create<mlir::LLVM::CallOp>(loc, fn, operands[0]);
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult GetIterLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                     mlir::ConversionPatternRewriter& rewriter) const {
    return linkOpToRuntimeFunc("pyir_getIter", op, operands, rewriter, 1);
}


mlir::LogicalResult ForIterLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                     mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::Location loc = op->getLoc();
    pyir::ForIter forIter = mlir::cast<pyir::ForIter>(op);

    // declare: extern Value* pyir_forIter(Value* iterator)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, getModule(op), "pyir_forIter", fnType);

    // Advance the iterator, returns nullptr if exhausted
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0]});
    mlir::Value next = call.getResult();

    // Check if result is null: icmp ne %next, null
    mlir::Value null = rewriter.create<mlir::LLVM::ZeroOp>(loc, ptrType(ctx));
    mlir::Value cond = rewriter.create<mlir::LLVM::ICmpOp>(loc, mlir::LLVM::ICmpPredicate::ne, next, null);

    // Branch to body with next value, or to exit if exhausted
    mlir::Block* bodyBlock = forIter.getBody();
    mlir::Block* exitBlock = forIter.getExit();

    // Convert the body block argument type from !pyir.object to !llvm.ptr
    if (bodyBlock->getNumArguments() > 0) {
        mlir::BlockArgument arg = bodyBlock->getArgument(0);
        arg.setType(ptrType(ctx));
    }

    // Convert exit block argument types
    for (mlir::BlockArgument arg : exitBlock->getArguments())
        arg.setType(ptrType(ctx));

    // operands[0] is iterator, rest are exit_args
    mlir::ValueRange exitArgOperands = operands.drop_front(1);

    rewriter.replaceOpWithNewOp<mlir::LLVM::CondBrOp>(op, cond, bodyBlock, mlir::ValueRange{next}, exitBlock,
                                                      exitArgOperands);
    return mlir::success();
}
