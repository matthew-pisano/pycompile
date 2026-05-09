//
// Created by matthew on 3/24/26.
//

#include "lowering/builder_lowering.h"

/**
 * Helper function to build the array of PyObj* and call the runtime fn for building strings, lists, sets, or maps.
 * This abstracts the common logic of allocating an array, storing operands, and calling the runtime function with the
 * array pointer and count.
 */
mlir::LogicalResult buildArrayType(mlir::MLIRContext* ctx, mlir::Operation* op,
                                   const mlir::ArrayRef<mlir::Value> operands, const mlir::LLVM::LLVMFuncOp& fn,
                                   mlir::ConversionPatternRewriter& rewriter, const mlir::Location& loc) {
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
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern PyObj* pyir_buildString(Value** parts, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_buildString", fnType);
    return buildArrayType(ctx, op, operands, fn, rewriter, loc);
}


mlir::LogicalResult BuildListLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern PyObj* pyir_buildList(Value** parts, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_buildList", fnType);
    return buildArrayType(ctx, op, operands, fn, rewriter, loc);
}


mlir::LogicalResult ListExtendLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                        mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_listExtend(PyObj* list, PyObj* items)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_listExtend", fnType);

    // operands[0] = list, operands[1] = items
    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], operands[1]});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult ListAppendLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                        mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_listAppend(PyObj* list, PyObj* item)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_listAppend", fnType);

    // operands[0] = list, operands[1] = item
    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], operands[1]});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult BuildSetLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern PyObj* pyir_buildSet(Value** parts, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_buildSet", fnType);
    return buildArrayType(ctx, op, operands, fn, rewriter, loc);
}


mlir::LogicalResult SetUpdateLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_setUpdate(PyObj* set, PyObj* items)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_setUpdate", fnType);

    // operands[0] = set, operands[1] = items
    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], operands[1]});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult SetAddLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                    mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_setAdd(PyObj* set, PyObj* item)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_setAdd", fnType);

    // operands[0] = set, operands[1] = item
    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], operands[1]});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult BuildMapLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern PyObj* pyir_buildMap(Value** parts, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_buildMap", fnType);
    return buildArrayType(ctx, op, operands, fn, rewriter, loc);
}


mlir::LogicalResult MapAddLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                    mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_mapAdd(PyObj* dict, PyObj* key, PyObj* value)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
            mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_mapAdd", fnType);

    // operands[0] = map, operands[1] = key, operands[2] = value
    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], operands[1], operands[2]});
    rewriter.eraseOp(op);
    return mlir::success();
}
