#include <CLI/CLI.hpp>
#include <Python.h>
#include <filesystem>
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <mlir/IR/OwningOpRef.h>

#include "bytecode/bytecode.h"
#include "bytecode/serialize_bytecode.h"
#include "conversion/pyir_codegen.h"
#include "lowering/llvm_export.h"
#include "lowering/pyir_lowering.h"
#include "utils.h"
#include "version.h"


/**
 * Serializes Python Bytecode modules in textual format.
 * @param bytecodeModules The bytecode modules to serialize.
 * @param filename The filename to write to if a single module. Otherwise, names will be assumed.
 */
void exportByteCode(const std::vector<ByteCodeModule>& bytecodeModules, const std::string& filename = "") {
    for (const ByteCodeModule& bytecodeModule : bytecodeModules) {
        std::string moduleFileName = filename;
        if (bytecodeModules.size() > 1 || filename.empty()) {
            const std::filesystem::path modulePath(bytecodeModule.filename);
            moduleFileName = modulePath.filename().replace_extension(".byc");
        }

        std::stringstream ss;
        serializeByteCodeModule(bytecodeModule, ss);
        writeFileString(moduleFileName, ss.str());
    }
}


/**
 * Serializes MLIR in textual format.
 * @param mlirModules The MLIR modules to serialize.
 * @param filename The filename to write to if a single module. Otherwise, names will be assumed.
 */
void exportMLIRModules(const std::vector<mlir::OwningOpRef<mlir::ModuleOp>>& mlirModules,
                       const std::string& filename = "") {
    for (const mlir::OwningOpRef<mlir::ModuleOp>& mlirModule : mlirModules) {
        std::string moduleFileName = filename;
        if (mlirModules.size() > 1 || filename.empty()) {
            const std::filesystem::path modulePath(getMLIRModuleName(mlirModule));
            moduleFileName = modulePath.filename().replace_extension(".mlir");
        }

        std::string mlirModuleContent;
        llvm::raw_string_ostream llvmOs(mlirModuleContent);
        const mlir::OpPrintingFlags flags;
        mlirModule.get().getOperation()->print(llvmOs, flags);
        writeFileString(moduleFileName, mlirModuleContent);
    }
}

/**
 * Serializes LLVM IR in textual format.
 * @param llvmModule The LLVM IR module to print.
 * @param filename The filename to write to if a single module. Otherwise, names will be assumed.
 */
void exportLLVMIR(const std::unique_ptr<llvm::Module>& llvmModule, const std::string& filename = "") {
    std::string moduleFileName = filename;
    if (filename.empty()) {
        const std::filesystem::path modulePath(llvmModule->getName().data());
        moduleFileName = modulePath.filename().replace_extension(".ll");
    }

    std::string mlirModuleContent;
    llvm::raw_string_ostream llvmOs(mlirModuleContent);
    llvmModule->print(llvmOs, nullptr);
    writeFileString(moduleFileName, mlirModuleContent);
}


int main(const int argc, char* argv[]) {
    const std::string name = "pycompile";
    const std::string version = name + " " + Version::VERSION;

    std::vector<std::string> inputFileNames;
    bool upToPreprocess = false;
    bool upToCompile = false;
    bool upToLower = false;
    bool saveAllTemps = false;
    bool debugBuild = false;
    std::string outputFileName;
    std::string tmpFilesName;
    std::string optimLevel;
    CLI::App app{version + " - Python Compiler", name};
    app.add_option("file", inputFileNames, "A Python file")->required()->allow_extra_args();
    app.add_flag("-E", upToPreprocess, "Preprocess only; do not compile, lower, or link");
    app.add_flag("-S", upToCompile, "Compile only; do not lower, or link");
    app.add_flag("-c", upToLower, "Compile and lower, but do not link");
    app.add_flag("--save-temps", saveAllTemps, "Do not delete intermediate files");
    app.add_option("-o", outputFileName, "The name of the output file");
    app.add_option("-O", optimLevel, "The optimization level of the binary");
    app.add_flag("-g", debugBuild, "Whether to generate a debug build");
    app.set_version_flag("--version", version);

    app.callback([&] {
        if (!saveAllTemps)
            tmpFilesName = outputFileName;
        else {
            upToPreprocess = true;
            upToCompile = true;
            upToLower = true;
        }

        if (!tmpFilesName.empty() && inputFileNames.size() > 1 && (upToPreprocess || upToCompile || upToLower))
            throw CLI::ValidationError("Cannot specify -o with -c, -S or -E with multiple files");
    });

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    // Ensure the directory of the output file exists
    if (!outputFileName.empty()) {
        std::filesystem::path outputFilePath(outputFileName);
        std::string outputDirName = std::filesystem::absolute(outputFilePath).parent_path();
        if (!std::filesystem::is_directory(outputDirName)) {
            std::cerr << "error: Could not find directory '" << outputDirName << "'" << std::endl;
            return 1;
        }
    }

    std::vector<std::string> fileContents;
    for (const std::string& fileName : inputFileNames) {
        try {
            fileContents.push_back(readFileString(fileName));
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << std::endl;
            return 1;
        }
    }

    // Compile source to Python bytecode
    std::vector<ByteCodeModule> bytecodeModules;
    try {
        bytecodeModules = compilePython(fileContents, inputFileNames);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (upToPreprocess) {
        exportByteCode(bytecodeModules, tmpFilesName);
        if (!saveAllTemps)
            return 0;
    }

    // Lower bytecode into PYIR MLIR dialect
    mlir::MLIRContext context; // Must exit scope after all other MLIR instances
    std::vector<mlir::OwningOpRef<mlir::ModuleOp>> mlirModules;
    for (const ByteCodeModule& module : bytecodeModules) {
        try {
            mlirModules.push_back(generatePyIR(context, module));
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }
    if (upToCompile) {
        exportMLIRModules(mlirModules, tmpFilesName);
        if (!saveAllTemps)
            return 0;
    }

    mlir::OwningOpRef<mlir::ModuleOp> mergedMlirModule;
    if (mlirModules.size() > 1)
        try {
            mergedMlirModule = mergePyIRModules(context, mlirModules);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    else
        mergedMlirModule = std::move(mlirModules[0]);

    // Lower PYIR to an LLVM MLIR dialect
    try {
        lowerToLLVMDialect(context, mergedMlirModule);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    LLVMExportOptions llvmOptions;
    llvmOptions.optLevel = std::stoi(optimLevel);

    llvm::LLVMContext llvmCtx; // Must exit scope after all other LLVM Module instances
    std::unique_ptr<llvm::Module> llvmModule;
    try {
        llvmModule = translateToLLVMIR(llvmCtx, mergedMlirModule, llvmOptions);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (upToLower) {
        exportLLVMIR(llvmModule, tmpFilesName);
        if (!saveAllTemps)
            return 0;
    }

    // Create object file
    std::filesystem::path modulePath(getMLIRModuleName(mergedMlirModule));
    const std::string moduleObjectPath = modulePath.replace_extension(".o");
    try {
        exportObjectFile(llvmModule, moduleObjectPath, llvmOptions);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Link object file into executable
    try {
        linkObjectFile(moduleObjectPath, outputFileName.empty() ? "a.out" : outputFileName);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Remove object file unless requested to keep
    if (!upToLower)
        try {
            if (!std::filesystem::remove(moduleObjectPath)) {
                std::cerr << "error: Failed to delete object file '" << moduleObjectPath << "'" << std::endl;
                return 1;
            }
        } catch (const std::filesystem::filesystem_error& ex) {
            std::cerr << "error: Failed to delete object file: " << ex.what() << std::endl;
            return 1;
        }

    return 0;
}
