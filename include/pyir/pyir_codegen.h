//
// Created by matthew on 3/6/26.
//

#ifndef PYCOMPILE_PYIR_CODEGEN_H
#define PYCOMPILE_PYIR_CODEGEN_H

#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/MLIRContext.h>

#include "bytecode/bytecode.h"
#include "pyir/pyir_ops.h"

namespace pyir {
    /**
     * Error to signal failure to parse bytecode into PyIR.
     */
    class PyIRError : public std::exception {
    public:
        PyIRError(const std::string& msg, const std::string& moduleName, const size_t lineno, const size_t offset) {
            this->msg = std::format("{}:{}:{}: error: {}", moduleName, lineno, offset, msg);
        }

        PyIRError(const std::string& msg, const std::string& moduleName, const std::optional<size_t> lineno,
                  const size_t offset) {
            size_t resolvedLineno = lineno.has_value() ? *lineno : 0;
            this->msg = std::format("{}:{}:{}: error: {}", moduleName, resolvedLineno, offset, msg);
        }

        const char* what() const noexcept override {
            return msg.c_str();
        }

    private:
        std::string msg;
    };


    // Wraps a single module into an mlir::ModuleOp containing one FuncOp
    mlir::OwningOpRef<mlir::ModuleOp> generatePyIR(mlir::MLIRContext& ctx, const ByteCodeModule& module);

    // Wraps multiple modules into a single mlir::ModuleOp
    mlir::OwningOpRef<mlir::ModuleOp> mergePyIRModules(mlir::MLIRContext& ctx,
                                                       std::vector<mlir::OwningOpRef<mlir::ModuleOp> >& mlirModules);

    void printMLIRFuncOp(mlir::func::FuncOp fn, std::ostream& os);

    void serializePyIRModule(const mlir::ModuleOp& module, std::ostream& os);
} //namespace pyir

#endif //PYCOMPILE_PYIR_CODEGEN_H
