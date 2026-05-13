//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_MEMORY_LOWERING_H
#define PYCOMPILE_MEMORY_LOWERING_H

#include "lowering/pyir_conversion_utils.hpp"
#include "pyir/pyir_ops.hpp"


/**
 * Lowers pyir.init_module to a call to the runtime function pyir_initModule.
 *
 * The file and name strings are stored as global constants and passed as const char* pointers.
 * The runtime pre-populates the module scope with dunder variables such as __name__ and __file__.
 *
 * pyir.init_module "<embedded>", "__main__"
 *     %file_ptr = llvm.mlir.addressof @__pyir_str___pymodule_file__
 *     %name_ptr = llvm.mlir.addressof @__pyir_str___pymodule_name__
 *                 llvm.call @pyir_initModule(%file_ptr, %name_ptr)
 */
struct InitModuleLowering : PyIROpConversion {
    InitModuleLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::InitModule::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.destroy_module to a call to the runtime function pyir_destroyModule.
 *
 * pyir.destroy_module
 */
struct DestroyModuleLowering : PyIROpConversion {
    DestroyModuleLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::DestroyModule::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_fast to a call to the runtime function pyir_loadFast.
 *
 * The name string is stored as a constant and passed as a const char* pointer. The runtime resolves the name
 * against the names present in the current scope and returns a heap-allocated PyObj*.
 *
 * pyir.load_fast "arg"
 *     %ptr = llvm.mlir.addressof @__pyir_str_arg
 *     %val = llvm.call @pyir_loadFast(%ptr)
 */
struct LoadFastLowering : PyIROpConversion {
    LoadFastLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadFast::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_fast_and_clear to a call to the runtime function pyir_loadFastAndClear.
 *
 * The name string is stored as a constant and passed as a const char* pointer. The runtime resolves the name
 * against the names present in the current scope and clears the variable with that name.
 *
 * pyir.load_fast_and_clear "arg"
 *     %ptr = llvm.mlir.addressof @__pyir_str_arg
 *     %val = llvm.call @pyir_loadFastAndClear(%ptr)
 */
struct LoadFastAndClearLowering : PyIROpConversion {
    LoadFastAndClearLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadFastAndClear::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.store_fast to a call to the runtime function pyir_storeFast.
 *
 * The name string is stored as a constant and passed as a const char* pointer alongside the heap-allocated
 * PyObj* to store. The runtime inserts or replaces the name in a local scope table, managing refcounts on the old
 * and new values.
 *
 * pyir.store_fast "x", %val
 *   %ptr = llvm.mlir.addressof @__pyir_str_x
 *          llvm.call @pyir_storeFast(%ptr, %val)
 */
struct StoreFastLowering : PyIROpConversion {
    StoreFastLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::StoreFast::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_name to a call to the runtime function pyir_loadName.
 *
 * The name string is stored as a global constant and passed as a const char* pointer. The runtime resolves the name
 * against the builtin table or module names and returns a heap-allocated PyObj*.
 *
 * pyir.load_name "print"
 *     %ptr = llvm.mlir.addressof @__pyir_str_print
 *     %val = llvm.call @pyir_loadName(%ptr)
 */
struct LoadNameLowering : PyIROpConversion {
    LoadNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadName::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.store_name to a call to the runtime function pyir_storeName.
 *
 * The name string is stored as a global constant and passed as a const char* pointer alongside the heap-allocated
 * PyObj* to store. The runtime inserts or replaces the name in the module scope table, managing refcounts on the old
 * and new values.
 *
 * pyir.store_name "a", %val
 *     %ptr = llvm.mlir.addressof @__pyir_str_a
 *            llvm.call @pyir_storeName(%ptr, %val)
 */
struct StoreNameLowering : PyIROpConversion {
    StoreNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::StoreName::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_attr to a call to the runtime function pyir_load_attr.
 *
 * The attribute name is stored as a global string constant and passed as a const char* pointer
 * alongside the heap-allocated PyObj* object. The runtime looks up the attribute on the object
 * and returns a heap-allocated PyObj* (typically a BoundMethod).
 *
 * pyir.load_attr %obj, "append" : !pyir.object
 *     %name_ptr = llvm.mlir.addressof @__pyir_str_append
 *     %result   = llvm.call @pyir_loadAttr(%obj, %name_ptr)
 */
struct LoadAttrLowering : PyIROpConversion {
    LoadAttrLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadAttr::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.load_const to a call to the appropriate runtime constant constructor, depending on the attribute type:
 *
 *   StringAttr -> pyir_loadConstStr(const char*)
 *   IntegerAttr -> pyir_loadConstInt(int64_t)
 *
 * String constants are stored as global i8 arrays. Integer constants are passed directly as i64 values. Both return a
 * heap-allocated PyObj*.
 */
struct LoadConstLowering : PyIROpConversion {
    LoadConstLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadConst::getOperationName(), tc, ctx) {}

    /**
     * Helper function for lowering the code for loading a literal string constant.
     */
    static mlir::Value loadStringConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                       const mlir::Location& loc, const mlir::ModuleOp& module,
                                       const mlir::StringAttr& strAttr);

    /**
     * Helper function for lowering the code for loading a literal bool constant.
     */
    static mlir::Value loadBoolConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                     const mlir::Location& loc, const mlir::ModuleOp& module,
                                     const mlir::BoolAttr& boolAttr);

    /**
     * Helper function for lowering the code for loading a literal int constant.
     */
    static mlir::Value loadIntConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                    const mlir::Location& loc, const mlir::ModuleOp& module,
                                    const mlir::IntegerAttr& intAttr);

    /**
     * Helper function for lowering the code for loading a literal float constant.
     */
    static mlir::Value loadFloatConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                      const mlir::Location& loc, const mlir::ModuleOp& module,
                                      const mlir::FloatAttr& floatAttr);

    /**
     * Helper function for lowering the code for loading a literal None constant.
     */
    static mlir::Value loadNoneConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                     const mlir::Location& loc, const mlir::ModuleOp& module);

    /**
     * Helper function for lowering the code for loading a literal tuple constant.
     */
    static mlir::Value loadTupleConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                      const mlir::Location& loc, const mlir::ModuleOp& module,
                                      const mlir::ArrayAttr& arrAttr);

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
    LoadArgLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadArg::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};


/**
 * Lowers pyir.store_subscr to a call to the runtime function pyir_storeSubscr.
 *
 * Stores a heap-allocated PyObj* into a collection at the given index.
 * Both the collection and index are heap-allocated PyObj* pointers.
 *
 * pyir.store_subscr %collection, %idx, %value : !pyir.object, !pyir.object, !pyir.object
 *     llvm.call @pyir_storeSubscr(%collection, %idx, %value)
 */
struct StoreSubscrLowering : PyIROpConversion {
    StoreSubscrLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::StoreSubscr::getOperationName(), tc, ctx) {}

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override;
};

#endif // PYCOMPILE_MEMORY_LOWERING_H
