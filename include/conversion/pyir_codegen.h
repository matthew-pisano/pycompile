//
// Created by matthew on 3/6/26.
//

#ifndef PYCOMPILE_PYIR_CODEGEN_H
#define PYCOMPILE_PYIR_CODEGEN_H

#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/MLIRContext.h>

#include "bytecode/bytecode.h"

namespace pyir {

    // Wraps a single module into an mlir::ModuleOp containing one FuncOp
    mlir::OwningOpRef<mlir::ModuleOp> generatePyIR(mlir::MLIRContext& ctx, const ByteCodeModule& module);

    // Wraps multiple modules into a single mlir::ModuleOp
    mlir::OwningOpRef<mlir::ModuleOp> mergePyIRModules(mlir::MLIRContext& ctx,
                                                       std::vector<mlir::OwningOpRef<mlir::ModuleOp> >& mlirModules);
} //namespace pyir

#endif //PYCOMPILE_PYIR_CODEGEN_H
