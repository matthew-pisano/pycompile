//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_CODEGEN_UTILS_H
#define PYCOMPILE_CODEGEN_UTILS_H
#include <vector>


/**
 * A collection of data structures used for bookkeeping during the bytecode to MLIR conversion process
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

#endif // PYCOMPILE_CODEGEN_UTILS_H
