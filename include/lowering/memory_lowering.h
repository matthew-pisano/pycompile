//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_MEMORY_LOWERING_H
#define PYCOMPILE_MEMORY_LOWERING_H

#include "pyir/pyir_ops.h"
#include "lowering/pyir_conversion_utils.h"


/**
 * Lowers pyir.load_fast to a call to the runtime function pyir_loadFast.
 *
 * The name string is stored as a constant and passed as a const char* pointer. The runtime resolves the name
 * against the names present in the current scope and returns a heap-allocated Value*.
 *
 * pyir.load_fast "arg"
 *     %ptr = llvm.mlir.addressof @__pyir_str_arg
  *    %val = llvm.call @pyir_loadFast(%ptr)
 */
struct LoadFastLowering : PyIROpConversion {
    LoadFastLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::LoadFast::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.store_fast to a call to the runtime function pyir_storeFast.
 *
 * The name string is stored as a constant and passed as a const char* pointer alongside the heap-allocated
 * Value* to store. The runtime inserts or replaces the name in a local scope table, managing refcounts on the old
 * and new values.
 *
 * pyir.store_fast "x", %val
 *   %ptr = llvm.mlir.addressof @__pyir_str_x
 *          llvm.call @pyir_storeFast(%ptr, %val)
 */
struct StoreFastLowering : PyIROpConversion {
    StoreFastLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::StoreFast::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_name to a call to the runtime function pyir_loadName.
 *
 * The name string is stored as a global constant and passed as a const char* pointer. The runtime resolves the name
 * against the builtin table or module names and returns a heap-allocated Value*.
 *
 * pyir.load_name "print"
 *     %ptr = llvm.mlir.addressof @__pyir_str_print
 *     %val = llvm.call @pyir_loadName(%ptr)
 */
struct LoadNameLowering : PyIROpConversion {
    LoadNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::LoadName::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.store_name to a call to the runtime function pyir_storeName.
 *
 * The name string is stored as a global constant and passed as a const char* pointer alongside the heap-allocated
 * Value* to store. The runtime inserts or replaces the name in the module scope table, managing refcounts on the old
 * and new values.
 *
 * pyir.store_name "a", %val
 *     %ptr = llvm.mlir.addressof @__pyir_str_a
 *            llvm.call @pyir_storeName(%ptr, %val)
 */
struct StoreNameLowering : PyIROpConversion {
    StoreNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::StoreName::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_const to a call to the appropriate runtime constant constructor, depending on the attribute type:
 *
 *   StringAttr -> pyir_loadConstStr(const char*)
 *   IntegerAttr -> pyir_loadConstInt(int64_t)
 *
 * String constants are stored as global i8 arrays. Integer constants are passed directly as i64 values. Both return a
 * heap-allocated Value*.
 */
struct LoadConstLowering : PyIROpConversion {
    LoadConstLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::LoadConst::getOperationName(), tc, ctx) {
    }

    static mlir::Value loadStringConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                       const mlir::Location& loc, const mlir::ModuleOp& module,
                                       const mlir::StringAttr& strAttr);

    static mlir::Value loadBoolConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                     const mlir::Location& loc, const mlir::ModuleOp& module,
                                     const mlir::BoolAttr& boolAttr);

    static mlir::Value loadIntConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                    const mlir::Location& loc, const mlir::ModuleOp& module,
                                    const mlir::IntegerAttr& intAttr);

    static mlir::Value loadFloatConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                      const mlir::Location& loc, const mlir::ModuleOp& module,
                                      const mlir::FloatAttr& floatAttr);

    static mlir::Value loadNoneConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                     const mlir::Location& loc, const mlir::ModuleOp& module);

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_arg to a GEP + load into the args array.
 *
 * pyir.load_arg %args_ptr[%index]
 *     %gep = llvm.gep %args_ptr[index]
 *     %val = llvm.load %gep
 */
struct LoadArgLowering : PyIROpConversion {
    LoadArgLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : PyIROpConversion(pyir::LoadArg::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif //PYCOMPILE_MEMORY_LOWERING_H