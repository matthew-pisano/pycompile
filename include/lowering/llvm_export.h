//
// Created by matthew on 3/9/26.
//

#ifndef PYCOMPILE_LLVM_EXPORT_H
#define PYCOMPILE_LLVM_EXPORT_H

#include <mlir/IR/BuiltinOps.h>

#include <filesystem>
#include <string>

/**
 * Options controlling how LLVM IR is exported from an MLIR module.
 */
struct LLVMExportOptions {
    /// The target triple to compile for (e.g. "x86_64-pc-linux-gnu"). Defaults to the host triple if empty.
    std::string targetTriple;

    /// The CPU to optimize for (e.g. "native", "skylake"). Defaults to "native" if empty.
    std::string cpu = "native";

    /// Optimization level: 0 = none, 1 = less, 2 = default, 3 = aggressive.
    unsigned int optLevel = 0;
};


/**
 * Translates a fully lowered MLIR module (containing only LLVM dialect ops) to LLVM IR and prints it to stdout
 *
 * Useful for inspecting the generated IR before compiling to an object file.
 *
 * @param module The MLIR module to translate. Must contain only LLVM dialect ops.
 * @param options Export options controlling target and optimization level.
 */
void exportLLVMIR(mlir::ModuleOp module, const LLVMExportOptions& options = {});


/**
 * Translates a fully lowered MLIR module to an object file at the given path.
 *
 * @param module The MLIR module to compile. Must contain only LLVM dialect ops.
 * @param output Path to write the object file to (e.g. "output.o").
 * @param options Export options controlling target and optimization level.
 * @throws std::runtime_error if translation or compilation fails.
 */
void exportObjectFile(mlir::ModuleOp module, const std::filesystem::path& output,
                      const LLVMExportOptions& options = {});

#endif //PYCOMPILE_LLVM_EXPORT_H
