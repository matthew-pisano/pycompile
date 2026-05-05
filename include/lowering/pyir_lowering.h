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
 * Lowers the PYIR module to a dialect of LLVM
 * @param ctx The MLIR context
 * @param module The module to lower
 */
void lowerToLLVMDialect(mlir::MLIRContext& ctx, const mlir::OwningOpRef<mlir::ModuleOp>& module);

#endif // PYCOMPILE_PYIR_LOWERING_H
