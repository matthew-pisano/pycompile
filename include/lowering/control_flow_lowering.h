//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_CONTROL_FLOW_LOWERING_H
#define PYCOMPILE_CONTROL_FLOW_LOWERING_H

#include "pyir/pyir_ops.h"
#include "lowering/pyir_conversion_utils.h"


/**
 * Lowers pyir.push_null to a call to pyir_pushNull().
 *
 * push_null is a CPython calling convention artifact that places a null sentinel below the callee on the stack. In the
 * lowered IR it becomes a runtime call that returns a null Value* sentinel, which the Call lowering discards.
 */
struct PushNullLowering : PyIROpConversion {
    PushNullLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::PushNull::getOperationName(), tc, ctx) {
    }

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
    PopTopLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::PopTop::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif //PYCOMPILE_CONTROL_FLOW_LOWERING_H