//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_CONTROL_FLOW_LOWERING_H
#define PYCOMPILE_CONTROL_FLOW_LOWERING_H

#include "lowering/pyir_conversion_utils.h"
#include "pyir/pyir_ops.h"


/**
 * Lowers pyir.push_null to a call to pyir_pushNull().
 *
 * push_null is a CPython calling convention artifact that places a null sentinel below the callee on the stack. In the
 * lowered IR it becomes a runtime call that returns a null Value* sentinel, which the Call lowering discards.
 */
struct PushNullLowering : PyIROpConversion {
    PushNullLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PushNull::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pop_top by erasing it.
 *
 * pop_top discards the top of the Python evaluation stack. In SSA form the value simply has no uses, so the op itself
 * can be erased. DCE will subsequently remove the producing op if it is Pure and has no other uses.
 */
struct PopTopLowering : PyIROpConversion {
    PopTopLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PopTop::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.get_iter to a call to the runtime function pyir_getIter.
 *
 * Wraps a heap-allocated container Value* in an iterator Value* that tracks
 * the current position for use with pyir.for_iter.
 *
 * pyir.get_iter %container : !pyir.object -> !pyir.object
 *     %result = llvm.call @pyir_getIter(%container)
 */
struct GetIterLowering : PyIROpConversion {
    GetIterLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::GetIter::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.for_iter to a call to the runtime function pyir_forIter followed by a conditional branch.
 *
 * Advances the iterator by one step. If the iterator is not exhausted, the next value is passed
 * as a block argument to the body block. If exhausted, branches to the exit block.
 *
 * pyir.for_iter %iterator -> body ^bb1, exit ^bb2 : !pyir.object
 *     %next = llvm.call @pyir_forIter(%iterator)
 *     %cond = llvm.icmp ne %next, null
 *             llvm.cond_br %cond, ^bb1(%next), ^bb2()
 */
struct ForIterLowering : PyIROpConversion {
    ForIterLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ForIter::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif // PYCOMPILE_CONTROL_FLOW_LOWERING_H
