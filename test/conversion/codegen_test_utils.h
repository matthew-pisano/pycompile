//
// Created by matthew on 3/25/26.
//

#ifndef PYCOMPILE_CODEGEN_TEST_UTILS_H
#define PYCOMPILE_CODEGEN_TEST_UTILS_H


#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/IR/OwningOpRef.h>

#include "conversion/pyir_codegen.h"


/**
 * Fixture that initializes the MLIR context and compiles python strings to MLIR modules
 */
struct MLIRFixture {
    mlir::MLIRContext mlirCtx;

    mlir::OwningOpRef<mlir::ModuleOp> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        mlir::OwningOpRef<mlir::ModuleOp> mlirModule = generatePyIR(mlirCtx, bytecodeModule);
        mlirModule.get().getOperation()->print(llvm::errs());
        return mlirModule;
    }
};


/**
 * Gets the MLIR operation at the given index
 */
inline mlir::Operation* getOp(mlir::func::FuncOp fn, const int index) {
    auto it = fn.getBlocks().front().getOperations().begin();
    std::advance(it, index);
    return &*it;
}

#endif // PYCOMPILE_CODEGEN_TEST_UTILS_H
