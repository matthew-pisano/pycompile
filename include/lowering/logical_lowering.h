//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_LOGICAL_LOWERING_H
#define PYCOMPILE_LOGICAL_LOWERING_H

#include "pyir/pyir_ops.h"
#include "lowering/pyir_conversion_utils.h"


/**
 * Lowers pyir.is_truthy to a call to the runtime function pyir_is_truthy.
 *
 * Evaluates a heap-allocated Value* as a literal boolean.
 *
 * pyir.is_truthy %val : !pyir.object
 *     %result = llvm.call @pyir_is_truthy(%val)
 */
struct IsTruthyLowering : PyIROpConversion {
    IsTruthyLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::IsTruthy::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.to_bool to a call to the runtime function pyir_to_bool.
 *
 * Coerces a heap-allocated Value* to a boolean Value* by delegating to the runtime toBool helper. The result is a
 * freshly allocated Value(bool).
 *
 * pyir.to_bool %val : !pyir.object
 *     %result = llvm.call @pyir_to_bool(%val)
 */
struct ToBoolLowering : PyIROpConversion {
    ToBoolLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ToBool::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_unary_invert to a call to the runtime function pyir_unary_invert.
 *
 * Inverts a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unary_invert %val : !pyir.object
 *     %result = llvm.call @pyir_unary_invert(%val)
 */
struct UnaryInvertLowering : PyIROpConversion {
    UnaryInvertLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryInvert::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_unary_negative to a call to the runtime function pyir_unary_negative.
 *
 * Negates a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unary_negative %val : !pyir.object
 *     %result = llvm.call @pyir_unary_negative(%val)
 */
struct UnaryNegativeLowering : PyIROpConversion {
    UnaryNegativeLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryNegative::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_unary_not to a call to the runtime function pyir_unary_not.
 *
 * Negates a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unary_not %val : !pyir.object
 *     %result = llvm.call @pyir_unary_not(%val)
 */
struct UnaryNotLowering : PyIROpConversion {
    UnaryNotLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryNot::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.binary_op to a call to the appropriate runtime binary operator function.
 *
 * The operator string is mapped to a runtime function at compile time. Both operands are heap-allocated Value*
 * pointers. The runtime performs the operation and returns a new heap-allocated Value*.
 *
 * pyir.binary_op "+", %lhs, %rhs
 *     %result = llvm.call @pyir_add(%lhs, %rhs)
 */
struct BinaryOpLowering : PyIROpConversion {
    BinaryOpLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BinaryOp::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.compare_op to a call to the appropriate runtime compare operator function.
 *
 * The operator string is mapped to a runtime function at compile time. Both operands are heap-allocated Value*
 * pointers. The runtime performs the operation and returns a new heap-allocated Value*.
 *
 * pyir.compare_op "==", %lhs, %rhs
 *     %result = llvm.call @pyir_eq(%lhs, %rhs)
 */
struct CompareOpLowering : PyIROpConversion {
    CompareOpLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::CompareOp::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_format_simple to a call to the runtime function pyir_format_simple.
 *
 * Formats a heap-allocated Value* as a string and returns a new heap-allocated Value*.
 *
 * pyir.pyir_format_simple %val : !pyir.object
 *     %result = llvm.call @pyir_format_simple(%val)
 */
struct FormatSimpleLowering : PyIROpConversion {
    FormatSimpleLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::FormatSimple::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif //PYCOMPILE_LOGICAL_LOWERING_H
