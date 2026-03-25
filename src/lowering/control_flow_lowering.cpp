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
