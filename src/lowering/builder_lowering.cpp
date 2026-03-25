//
// Created by matthew on 3/24/26.
//

#include "lowering/builder_lowering.h"


mlir::LogicalResult BuildStringLowering::matchAndRewrite(mlir::Operation* op,
                                                         const mlir::ArrayRef<mlir::Value> operands,
                                                         mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern Value* pyir_buildString(Value** parts, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
        ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_buildString", fnType);

    const int64_t count = static_cast<int64_t>(operands.size());

    // Allocate a stack array of Value* to hold all string parts: Value*[count]
    mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), count);
    mlir::LLVM::ConstantOp allocSize = rewriter.create<mlir::LLVM::ConstantOp>(
        loc, i64Type(ctx), rewriter.getI64IntegerAttr(1));
    mlir::LLVM::AllocaOp alloca = rewriter.create<mlir::LLVM::AllocaOp>(loc, ptrType(ctx), arrType, allocSize);

    // Store each string part into the array in order
    for (int64_t i = 0; i < count; i++) {
        // Compute pointer to parts[i] via GEP (get element pointer)
        mlir::LLVM::ConstantOp idx = rewriter.create<mlir::LLVM::ConstantOp>(
            loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));

        mlir::LLVM::GEPOp gep = rewriter.create<mlir::LLVM::GEPOp>(
            loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});

        // Store the i-th operand (a Value*) into parts[i]
        rewriter.create<mlir::LLVM::StoreOp>(loc, operands[i], gep);
    }

    // Pass the array pointer and count to pyir_buildString
    mlir::LLVM::ConstantOp countVal = rewriter.create<mlir::LLVM::ConstantOp>(
        loc, i64Type(ctx), rewriter.getI64IntegerAttr(count));
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{alloca, countVal});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}