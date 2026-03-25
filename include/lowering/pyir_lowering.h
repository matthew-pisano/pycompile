//
// Created by matthew on 3/9/26.
//

#ifndef PYCOMPILE_PYIR_LOWERING_H
#define PYCOMPILE_PYIR_LOWERING_H

#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>

#include <mlir/Conversion/LLVMCommon/TypeConverter.h>


/**
 * Populates conversion patterns that lower PyIR ops to LLVM dialect ops.
 * @param patterns The patterns for conversion
 * @param typeConverter The type converter to use, must have PyIR type conversions registered via
 * addPyIRTypeConversions.
 */
void populatePyIRToLLVMPatterns(mlir::RewritePatternSet& patterns, mlir::LLVMTypeConverter& typeConverter);


/**
 * Creates a standalone pass that lowers all PyIR dialect ops to the LLVM dialect. After this pass runs, the module
 * contains only LLVM dialect ops.
 */
std::unique_ptr<mlir::Pass> createPyIRToLLVMPass();


/**
 * Lowers the PYIR module to a dialect of LLVM
 * @param ctx The MLIR context
 * @param module The module to lower
 */
void lowerToLLVMDialect(mlir::MLIRContext& ctx, const mlir::OwningOpRef<mlir::ModuleOp>& module);

#endif // PYCOMPILE_PYIR_LOWERING_H
