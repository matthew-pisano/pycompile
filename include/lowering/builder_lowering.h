//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_LOWERING_H
#define PYCOMPILE_BUILDER_LOWERING_H

#include "lowering/pyir_conversion_utils.h"
#include "pyir/pyir_ops.h"


/**
 * Lowers pyir.pyir_buildString to allocate memory and construct a new string using the runtime function
 * pyir_buildString
 *
 * Parts are stack-allocated as a Value*[] array and passed by pointer along with the part count.
 * The runtime concatenates all parts into a single string Value*.
 *
 * pyir.build_string %part0, %part1, ... : (!pyir.object, !pyir.object, ...) -> !pyir.object
 *     %arr   = llvm.alloca [n x !llvm.ptr]
 *     %gep0  = llvm.gep %arr[0]
 *              llvm.store %part0, %gep0
 *     %gep1  = llvm.gep %arr[1]
 *              llvm.store %part1, %gep1
 *     ...
 *     %result = llvm.call @pyir_buildString(%arr, n)
 */
struct BuildStringLowering : PyIROpConversion {
    BuildStringLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BuildString::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif // PYCOMPILE_BUILDER_LOWERING_H
