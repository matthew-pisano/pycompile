//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_FUNCTION_LOWERING_H
#define PYCOMPILE_FUNCTION_LOWERING_H

#include "lowering/pyir_conversion_utils.h"
#include "pyir/pyir_ops.h"


/**
 * Lowers pyir.call to a call to the runtime function pyir_call.
 *
 * Arguments are stack-allocated as a Value*[] array and passed by pointer along with the argument count. The callee is
 * passed as a Value* pointer.
 *
 * pyir.call %callee(%arg0, %arg1)
 *     %arr    = llvm.alloca [2 x !llvm.ptr]
 *     %gep0   = llvm.gep %arr[0]
 *               llvm.store %arg0, %gep0
 *     %gep1   = llvm.gep %arr[1]
 *               llvm.store %arg1, %gep1
 *     %result = llvm.call @pyir_call(%callee, %arr, 2)
 *
 * For zero-argument calls, a null pointer is passed instead of an array.
 */
struct CallLowering : PyIROpConversion {
    CallLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::Call::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.push_scope to a call to pyir_pushScope().
 *
 * pyir.push_scope
 *     llvm.call @pyir_pushScope()
 */
struct PushScopeLowering : PyIROpConversion {
    PushScopeLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PushScope::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pop_scope to a call to pyir_popScope().
 *
 * pyir.pop_scope
 *     llvm.call @pyir_popScope()
 */
struct PopScopeLowering : PyIROpConversion {
    PopScopeLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PopScope::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.make_function to a call to pyir_makeFunction(fn_ptr).
 *
 * Takes the symbol name stored in the attribute, gets its address as a function pointer,
 * and wraps it in a heap-allocated Value* callable.
 *
 * pyir.make_function "fn_name"
 *     %ptr = llvm.mlir.addressof @fn_name
 *     %val = llvm.call @pyir_makeFunction(%ptr)
 */
struct MakeFunctionLowering : PyIROpConversion {
    MakeFunctionLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::MakeFunction::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.return_value to llvm.return.
 *
 * pyir.return_value %val
 *     llvm.return %val
 */
struct ReturnValueLowering : PyIROpConversion {
    ReturnValueLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ReturnValue::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif // PYCOMPILE_FUNCTION_LOWERING_H
