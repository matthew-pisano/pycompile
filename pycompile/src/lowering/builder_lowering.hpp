//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_LOWERING_H
#define PYCOMPILE_BUILDER_LOWERING_H

#include "lowering/pyir_conversion_utils.hpp"
#include "pyir/pyir_ops.hpp"


/**
 * Lowers pyir.pyir_buildString to allocate memory and construct a new string using the runtime function
 * pyir_buildString
 *
 * Parts are stack-allocated as a PyObj*[] array and passed by pointer along with the part count.
 * The runtime concatenates all parts into a single string PyObj*.
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


/**
 * Lowers pyir.pyir_buildList to allocate memory and construct a new list using the runtime function pyir_buildList
 *
 * Parts are stack-allocated as a PyObj*[] array and passed by pointer along with the part count.
 * The runtime appends all parts into a single list PyObj*.
 *
 * pyir.build_list %part0, %part1, ... : (!pyir.object, !pyir.object, ...) -> !pyir.object
 *     %arr   = llvm.alloca [n x !llvm.ptr]
 *     %gep0  = llvm.gep %arr[0]
 *              llvm.store %part0, %gep0
 *     %gep1  = llvm.gep %arr[1]
 *              llvm.store %part1, %gep1
 *     ...
 *     %result = llvm.call @pyir_buildList(%arr, n)
 */
struct BuildListLowering : PyIROpConversion {
    BuildListLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BuildList::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.list_extend to a call to the runtime function pyir_listExtend.
 *
 * Extends a heap-allocated list PyObj* with the contents of another PyObj*.
 *
 * pyir.list_extend %list, %items : !pyir.object, !pyir.object
 *     llvm.call @pyir_listExtend(%list, %items)
 */
struct ListExtendLowering : PyIROpConversion {
    ListExtendLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ListExtend::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.list_append to a call to the runtime function pyir_listAppend.
 *
 * Appends a heap-allocated list PyObj* with another PyObj*.
 *
 * pyir.list_append %list, %item : !pyir.object, !pyir.object
 *     llvm.call @pyir_listAppend(%list, %item)
 */
struct ListAppendLowering : PyIROpConversion {
    ListAppendLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ListAppend::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

/**
 * Lowers pyir.pyir_buildSet to allocate memory and construct a new set using the runtime function pyir_buildSet
 *
 * Parts are stack-allocated as a PyObj*[] array and passed by pointer along with the part count.
 * The runtime appends all parts into a single set PyObj*.
 *
 * pyir.build_set %part0, %part1, ... : (!pyir.object, !pyir.object, ...) -> !pyir.object
 *     %arr   = llvm.alloca [n x !llvm.ptr]
 *     %gep0  = llvm.gep %arr[0]
 *              llvm.store %part0, %gep0
 *     %gep1  = llvm.gep %arr[1]
 *              llvm.store %part1, %gep1
 *     ...
 *     %result = llvm.call @pyir_buildSet(%arr, n)
 */
struct BuildSetLowering : PyIROpConversion {
    BuildSetLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BuildSet::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.set_update to a call to the runtime function pyir_setUpdate.
 *
 * Updates a heap-allocated set PyObj* with the contents of another PyObj*.
 *
 * pyir.set_update %list, %items : !pyir.object, !pyir.object
 *     llvm.call @pyir_setUpdate(%set, %items)
 */
struct SetUpdateLowering : PyIROpConversion {
    SetUpdateLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::SetUpdate::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.set_add to a call to the runtime function pyir_setAdd.
 *
 * Adds a heap-allocated PyObj* to a set PyObj*.
 *
 * pyir.set_add %set, %item : !pyir.object, !pyir.object
 *     llvm.call @pyir_setAdd(%set, %item)
 */
struct SetAddLowering : PyIROpConversion {
    SetAddLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::SetAdd::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.pyir_buildMap to allocate memory and construct a new map using the runtime function pyir_buildMap
 *
 * Parts are stack-allocated as a PyObj*[] array and passed by pointer along with the part count.
 * The runtime appends all pairs of parts into a single dict PyObj*.
 *
 * pyir.build_map %part0, %part1, ... : (!pyir.object, !pyir.object, ...) -> !pyir.object
 *     %arr   = llvm.alloca [n x !llvm.ptr]
 *     %gep0  = llvm.gep %arr[0]
 *              llvm.store %part0, %gep0
 *     %gep1  = llvm.gep %arr[1]
 *              llvm.store %part1, %gep1
 *     ...
 *     %result = llvm.call @pyir_buildMap(%arr, n)
 */
struct BuildMapLowering : PyIROpConversion {
    BuildMapLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BuildMap::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.map_add to a call to the runtime function pyir_mapAdd.
 *
 * Adds two heap-allocated PyObj* key and values to a dict PyObj*.
 *
 * pyir.map_add %map, %key, %value : !pyir.object, !pyir.object, !pyir.object
 *     llvm.call @pyir_mapAdd(%map, %key, %value)
 */
struct MapAddLowering : PyIROpConversion {
    MapAddLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::MapAdd::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


#endif // PYCOMPILE_BUILDER_LOWERING_H
