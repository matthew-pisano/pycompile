//
// Created by matthew on 3/24/26.
//

#include "builder_lowering.hpp"

/**
 * Helper function to build the array of PyObj* and call the runtime fn for building strings, lists, sets, or maps.
 * This abstracts the common logic of allocating an array, storing operands, and calling the runtime function with the
 * array pointer and count.
 */
mlir::LogicalResult buildArrayType(const std::string& func, mlir::Operation* op,
                                   const mlir::ArrayRef<mlir::Value> operands,
                                   mlir::ConversionPatternRewriter& rewriter) {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::Location loc = op->getLoc();
    const int64_t count = static_cast<int64_t>(operands.size());

    // Allocate a stack array of PyObj* to hold all parts: PyObj*[count]
    const mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), count);
    mlir::LLVM::ConstantOp allocSize =
            mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(1));
    mlir::LLVM::AllocaOp alloca = mlir::LLVM::AllocaOp::create(rewriter, loc, ptrType(ctx), arrType, allocSize);

    // Store each part into the array in order
    for (int64_t i = 0; i < count; i++) {
        // Compute pointer to parts[i] via GEP (get element pointer)
        mlir::LLVM::ConstantOp idx =
                mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));

        mlir::LLVM::GEPOp gep =
                mlir::LLVM::GEPOp::create(rewriter, loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});

        // Store the i-th operand (a PyObj*) into parts[i]
        mlir::LLVM::StoreOp::create(rewriter, loc, operands[i], gep);
    }

    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    const mlir::ModuleOp module = op->getParentOfType<mlir::ModuleOp>();
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, func, fnType);

    // Pass the array pointer and count to the called function
    mlir::LLVM::ConstantOp countVal =
            mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(count));
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{alloca, countVal});
    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult BuildStringLowering::matchAndRewrite(mlir::Operation* op,
                                                         const mlir::ArrayRef<mlir::Value> operands,
                                                         mlir::ConversionPatternRewriter& rewriter) const {
    return buildArrayType("pyir_buildString", op, operands, rewriter);
}


mlir::LogicalResult BuildListLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    return buildArrayType("pyir_buildList", op, operands, rewriter);
}


mlir::LogicalResult ListExtendLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                        mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_listExtend", op, operands, rewriter, false);
}


mlir::LogicalResult ListAppendLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                        mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_listAppend", op, operands, rewriter, false);
}


mlir::LogicalResult BuildSetLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    return buildArrayType("pyir_buildSet", op, operands, rewriter);
}


mlir::LogicalResult SetUpdateLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_setUpdate", op, operands, rewriter, false);
}


mlir::LogicalResult SetAddLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                    mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_setAdd", op, operands, rewriter, false);
}


mlir::LogicalResult BuildMapLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    return buildArrayType("pyir_buildMap", op, operands, rewriter);
}


mlir::LogicalResult MapAddLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                    mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_mapAdd", op, operands, rewriter, false);
}
