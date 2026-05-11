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
    return insertRuntimeFunc("pyir_decref", op, operands, rewriter, false);
}


mlir::LogicalResult GetIterLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                     mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_getIter", op, operands, rewriter);
}


mlir::LogicalResult ForIterLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                     mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::Location loc = op->getLoc();
    pyir::ForIter forIter = mlir::cast<pyir::ForIter>(op);

    // declare: extern PyObj* pyir_forIter(PyObj* iterator)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, getModule(op), "pyir_forIter", fnType);

    // Advance the iterator, returns nullptr if exhausted
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0]});
    const mlir::Value next = call.getResult();

    // Check if result is null: icmp ne %next, null
    const mlir::Value null = mlir::LLVM::ZeroOp::create(rewriter, loc, ptrType(ctx));
    mlir::Value cond = mlir::LLVM::ICmpOp::create(rewriter, loc, mlir::LLVM::ICmpPredicate::ne, next, null);

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


mlir::LogicalResult PopIterLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                     mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_popIter", op, operands, rewriter, false);
}
