//
// Created by matthew on 3/6/26.
//

#ifndef PYCOMPILE_PYIR_CODEGEN_H
#define PYCOMPILE_PYIR_CODEGEN_H

#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/MLIRContext.h>

#include "bytecode/bytecode.h"


/**
 * A collection of data structures used for bookkeeping during the bytecode to MLIR conversion process.
 */
struct ConversionMeta {
    /// Value stack, maps the CPython evaluation stack to SSA values.
    std::vector<mlir::Value> stack;
    /// Maps program offsets to program blocks, used for jump addressing.
    std::unordered_map<size_t, mlir::Block*> offsetToBlock;
    /// Keeps track of declared function names before construction.
    std::unordered_map<mlir::Value*, std::string> pendingFunctions;
    /// Whether the module is a top-level module or a function
    bool isFunction;
};


/**
 * Builds an MLIR module from the given bytecode.
 */
void buildMLIRModule(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module,
                     const std::string& moduleName);

/**
 * Wraps a single module into an mlir::ModuleOp containing one FuncOp.
 */
mlir::OwningOpRef<mlir::ModuleOp> generatePyIR(mlir::MLIRContext& ctx, const ByteCodeModule& module);

/**
 * Wraps multiple modules into a single mlir::ModuleOp.
 */
mlir::OwningOpRef<mlir::ModuleOp> mergePyIRModules(mlir::MLIRContext& ctx,
                                                   std::vector<mlir::OwningOpRef<mlir::ModuleOp>>& mlirModules);

#endif // PYCOMPILE_PYIR_CODEGEN_H
