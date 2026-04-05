//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_LOGICAL_LOWERING_H
#define PYCOMPILE_LOGICAL_LOWERING_H

#include "lowering/pyir_conversion_utils.h"
#include "pyir/pyir_ops.h"


/**
 * Lowers pyir.is_truthy to a call to the runtime function pyir_isTruthy.
 *
 * Evaluates a heap-allocated Value* as a literal boolean.
 *
 * pyir.is_truthy %val : !pyir.object
 *     %result = llvm.call @pyir_isTruthy(%val)
 */
struct IsTruthyLowering : PyIROpConversion {
    IsTruthyLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::IsTruthy::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.to_bool to a call to the runtime function pyir_toBool.
 *
 * Coerces a heap-allocated Value* to a boolean Value* by delegating to the runtime toBool helper. The result is a
 * freshly allocated Value(bool).
 *
 * pyir.to_bool %val : !pyir.object
 *     %result = llvm.call @pyir_toBool(%val)
 */
struct ToBoolLowering : PyIROpConversion {
    ToBoolLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ToBool::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_unaryInvert to a call to the runtime function pyir_unaryInvert.
 *
 * Inverts a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unaryInvert %val : !pyir.object
 *     %result = llvm.call @pyir_unaryInvert(%val)
 */
struct UnaryInvertLowering : PyIROpConversion {
    UnaryInvertLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryInvert::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_unaryNegative to a call to the runtime function pyir_unaryNegative.
 *
 * Negates a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unaryNegative %val : !pyir.object
 *     %result = llvm.call @pyir_unaryNegative(%val)
 */
struct UnaryNegativeLowering : PyIROpConversion {
    UnaryNegativeLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryNegative::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_unaryNot to a call to the runtime function pyir_unaryNot.
 *
 * Negates a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unaryNot %val : !pyir.object
 *     %result = llvm.call @pyir_unaryNot(%val)
 */
struct UnaryNotLowering : PyIROpConversion {
    UnaryNotLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryNot::getOperationName(), tc, ctx) {}

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
        PyIROpConversion(pyir::BinaryOp::getOperationName(), tc, ctx) {}

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
        PyIROpConversion(pyir::CompareOp::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.contains_op to a call to the appropriate runtime contains operator function.
 *
 * The operator string is mapped to a runtime function at compile time. Both operands are heap-allocated Value*
 * pointers. The runtime performs the operation and returns a new heap-allocated Value*.
 *
 * pyir.contains_op "==", %lhs, %rhs
 *     %result = llvm.call @pyir_in(%lhs, %rhs)
 */
struct ContainsOpLowering : PyIROpConversion {
    ContainsOpLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ContainsOp::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_formatSimple to a call to the runtime function pyir_formatSimple.
 *
 * Formats a heap-allocated Value* as a string and returns a new heap-allocated Value*.
 *
 * pyir.pyir_formatSimple %val : !pyir.object
 *     %result = llvm.call @pyir_formatSimple(%val)
 */
struct FormatSimpleLowering : PyIROpConversion {
    FormatSimpleLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::FormatSimple::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif // PYCOMPILE_LOGICAL_LOWERING_H
