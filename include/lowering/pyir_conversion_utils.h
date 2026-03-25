//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_PYIR_CONVERSION_UTILS_H
#define PYCOMPILE_PYIR_CONVERSION_UTILS_H

#include <mlir/IR/MLIRContext.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/Conversion/LLVMCommon/TypeConverter.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>

/**
 * Returns an opaque LLVM pointer type (!llvm.ptr) in the given context.
 * Used as the LLVM representation of pyir::Value* throughout the lowering.
 */
mlir::LLVM::LLVMPointerType ptrType(mlir::MLIRContext * ctx);


/**
 * Returns a bool (8-bit integer for LLVM) type in the given context.
 */
mlir::Type boolType(mlir::MLIRContext * ctx);


/**
 * Returns a 64-bit integer type in the given context.
 */
mlir::Type i64Type(mlir::MLIRContext * ctx);


/**
 * Returns an 8-bit integer type in the given context.
 */
mlir::Type i8Type(mlir::MLIRContext * ctx);


/**
 * Returns a 64-bit float type in the given context.
 */
mlir::Type f64Type(mlir::MLIRContext * ctx);


/**
 * Looks up an external runtime function by name in the module, inserting a declaration if one does not already exist.
 *
 * This is used to lazily declare runtime functions (e.g. pyir_loadName, pyir_call) only when an op that needs them is
 * encountered during lowering, avoiding unused declarations in the output.
 *
 * @param rewriter The pattern rewriter, used to insert the declaration.
 * @param module The module to insert the declaration into.
 * @param name The symbol name of the runtime function.
 * @param fnType The LLVM function type of the declaration.
 * @return The existing or newly created LLVMFuncOp declaration.
 */
mlir::LLVM::LLVMFuncOp getOrInsertRuntimeFn(mlir::PatternRewriter& rewriter, mlir::ModuleOp module,
                                            llvm::StringRef name, mlir::LLVM::LLVMFunctionType fnType);


/**
 * Creates a private global string constant in the module if one with the given name does not already exist, and
 * returns a pointer to it.
 *
 * The string is stored as a null-terminated i8 array in the module's rodata section. Identical strings are
 * deduplicated by name.
 *
 * @param rewriter The pattern rewriter, used to insert the global.
 * @param module The module to insert the global into.
 * @param loc The location to attach to the global op.
 * @param name The symbol name for the global.
 * @param str The string value to store, without null terminator.
 * @return An !llvm.ptr value pointing to the global.
 */
mlir::Value getOrInsertStringConstant(mlir::ConversionPatternRewriter& rewriter, mlir::ModuleOp module,
                                      mlir::Location loc, llvm::StringRef name, llvm::StringRef str);


/**
 * Base class for all PyIR to LLVM conversion patterns.
 *
 * Provides a convenience method for retrieving the parent ModuleOp, which is needed by patterns that insert global
 * declarations or runtime function declarations.
 */
struct PyIROpConversion : mlir::ConversionPattern {
    PyIROpConversion(llvm::StringRef opName, const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) : ConversionPattern(tc, opName, /*benefit=*/1, ctx) {
    }

    static mlir::ModuleOp getModule(mlir::Operation* op) {
        return op->getParentOfType<mlir::ModuleOp>();
    }

    /**
     * A helper function for directly linking ops to functions in the runtime standard library
     * @param func The name of the runtime function to link to
     * @param op The operation to be linked
     * @param operands The operands for the function
     * @param rewriter The MLIR rewriter
     * @param argc The number of arguments the op takes (e.g. binary or unary)
     * @return The status result
     */
    static mlir::LogicalResult linkOpToRuntimeFunc(const std::string& func, mlir::Operation* op,
                                                   mlir::ArrayRef<mlir::Value> operands,
                                                   mlir::ConversionPatternRewriter& rewriter, size_t argc);
};

#endif //PYCOMPILE_PYIR_CONVERSION_UTILS_H