//
// Created by matthew on 3/9/26.
//

#ifndef PYCOMPILE_LLVM_EXPORT_H
#define PYCOMPILE_LLVM_EXPORT_H

#include <mlir/IR/BuiltinOps.h>

#include <filesystem>
#include <llvm/IR/LLVMContext.h>
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

    /// Whether to generate a debug build.
    bool debugInfo = false;
};


/**
 * Translates a fully lowered MLIR module (containing LLVM dialect ops) to LLVM IR and prints it to the given stream
 *
 * @param llvmCtx The llvm context to use for translations
 * @param module The MLIR module to translate. Must contain only LLVM dialect ops.
 * @param options Export options controlling target and optimization level.
 */
std::unique_ptr<llvm::Module> translateToLLVMIR(llvm::LLVMContext& llvmCtx,
                                                const mlir::OwningOpRef<mlir::ModuleOp>& module,
                                                const LLVMExportOptions& options = {});


/**
 * Translates a fully lowered MLIR module to an object file at the given path.
 *
 * @param llvmModule The LLVM IR module to export
 * @param output Path to write the object file to (e.g. "output.o").
 * @param options Export options controlling target and optimization level.
 * @throws std::runtime_error if translation or compilation fails.
 */
void exportObjectFile(const std::unique_ptr<llvm::Module>& llvmModule, const std::filesystem::path& output,
                      const LLVMExportOptions& options = {});


/**
 * Links an object file with system libraries to create an executable
 * @param obj The path to the object file
 * @param output The path to the output file
 */
void linkObjectFile(const std::filesystem::path& obj, const std::filesystem::path& output);

#endif // PYCOMPILE_LLVM_EXPORT_H
