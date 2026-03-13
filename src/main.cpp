#include <csignal>
#include <CLI/CLI.hpp>
#include <Python.h>
#include <iostream>
#include <mlir/IR/OwningOpRef.h>

#include "bytecode/bytecode.h"
#include "bytecode/pythoncode.h"
#include "bytecode/python_raii.h"
#include "utils.h"
#include "version.h"
#include "lowering/llvm_export.h"
#include "lowering/pyir_to_llvm.h"
#include "pyir/pyir_codegen.h"


/**
 * Serializes Python Bytecode modules in textual format.
 * @param bytecodeModules The bytecode modules to serialize.
 * @param filename The filename to write to if a single module. Otherwise, names will be assumed.
 */
void exportByteCode(const std::vector<ByteCodeModule>& bytecodeModules, const std::string& filename = "") {
    for (const ByteCodeModule& bytecodeModule : bytecodeModules) {
        std::string moduleFileName = filename;
        if (bytecodeModules.size() > 1 || filename.empty()) {
            std::filesystem::path modulePath(bytecodeModule.filename);
            moduleFileName = modulePath.replace_extension(".byc");
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
void exportMLIRModules(const std::vector<mlir::OwningOpRef<mlir::ModuleOp> >& mlirModules,
                       const std::string& filename = "") {
    for (const mlir::OwningOpRef<mlir::ModuleOp>& mlirModule : mlirModules) {
        std::string moduleFileName = filename;
        if (mlirModules.size() > 1 || filename.empty()) {
            std::filesystem::path modulePath(getMLIRModuleName(mlirModule));
            moduleFileName = modulePath.replace_extension(".mlir");
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
void exportLLVMIR(const mlir::OwningOpRef<mlir::ModuleOp>& llvmModule, const std::string& filename = "") {
    std::string moduleFileName = filename;
    if (filename.empty()) {
        std::filesystem::path modulePath(getMLIRModuleName(llvmModule));
        moduleFileName = modulePath.replace_extension(".llvm");
    }

    std::string mlirModuleContent;
    llvm::raw_string_ostream llvmOs(mlirModuleContent);
    serializeLLVMIR(llvmModule, llvmOs);
    writeFileString(moduleFileName, mlirModuleContent);
}


std::vector<ByteCodeModule> compilePython(const std::vector<std::string>& fileContents,
                                          const std::vector<std::string>& fileNames) {
    PythonInterpreter pyInterp; // Initializes Python via RAII
    std::vector<CompiledModule> compiledModules;
    compiledModules.reserve(fileContents.size());
    for (size_t i = 0; i < fileContents.size(); i++)
        try {
            compiledModules.push_back(compilePythonSource(fileContents[0], fileNames[i], fileNames[i]));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error compiling Python file '" + fileNames[i] + "': " + e.what());
        }

    // Disassemble PyObjects into Python bytecode
    std::vector<ByteCodeModule> bytecodeModules;
    bytecodeModules.reserve(compiledModules.size());
    for (size_t i = 0; i < compiledModules.size(); i++)
        try {
            bytecodeModules.push_back(generatePythonBytecode(compiledModules[i]));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error generating bytecode for Python file '" + fileNames[i] + "': " + e.what());
        }

    return bytecodeModules;
}


int main(const int argc, char* argv[]) {
    const std::string name = "pycompile";
    const std::string version = name + " " + Version::VERSION;

    std::vector<std::string> inputFileNames;
    bool upToPreprocess = false;
    bool upToCompile = false;
    bool upToLower = false;
    std::string outputFileName;
    CLI::App app{version + " - Python Compiler", name};
    app.add_option("file", inputFileNames, "A Python file")->required()->allow_extra_args();
    app.add_flag("-E", upToPreprocess, "Preprocess only; do not compile, lower, or link.");
    app.add_flag("-S", upToCompile, "Compile only; do not lower, or link.");
    app.add_flag("-c", upToLower, "Compile and lower, but do not link.");
    app.add_option("-o", outputFileName, "A Python file");
    app.set_version_flag("--version", version);

    app.callback([&] {
        if (!outputFileName.empty() && inputFileNames.size() > 1 && (upToPreprocess || upToCompile || upToLower))
            throw CLI::ValidationError("Cannot specify -o with -c, -S or -E with multiple files");
    });

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    std::vector<std::string> fileContents;
    for (const std::string& fileName : inputFileNames) {
        try {
            fileContents.push_back(readFileString(fileName));
        } catch (const std::runtime_error& e) {
            std::cerr << "Error reading file '" + fileName + "': " + e.what() << std::endl;
            return 1;
        }
    }

    // Compile source to Python bytecode
    std::vector<ByteCodeModule> bytecodeModules;
    try {
        bytecodeModules = compilePython(fileContents, inputFileNames);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (upToPreprocess) {
        exportByteCode(bytecodeModules, outputFileName);
        return 0;
    }

    // Lower bytecode into PYIR MLIR dialect
    mlir::MLIRContext context; // Must exit scope after all other MLIR instances
    std::vector<mlir::OwningOpRef<mlir::ModuleOp> > mlirModules;
    for (const ByteCodeModule& module : bytecodeModules) {
        try {
            mlirModules.push_back(pyir::generatePyIR(context, module));
        } catch (const std::runtime_error& e) {
            std::cerr << "Error generating PyIR for file '" + module.filename + "': " + e.what() << std::endl;
            return 1;
        }
    }
    if (upToCompile) {
        exportMLIRModules(mlirModules, outputFileName);
        return 0;
    }

    mlir::OwningOpRef<mlir::ModuleOp> mergedMlirModule;
    if (mlirModules.size() > 1)
        try {
            mergedMlirModule = pyir::mergePyIRModules(context, mlirModules);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error merging PyIR modules: " << e.what() << std::endl;
            return 1;
        }
    else
        mergedMlirModule = std::move(mlirModules[0]);

    // Lower PYIR to an LLVM MLIR dialect
    try {
        lowerToLLVM(context, mergedMlirModule.get());
    } catch (const std::runtime_error& e) {
        std::cerr << "Error lowering mlir module: " << e.what() << std::endl;
        return 1;
    }
    if (upToLower) {
        exportLLVMIR(mergedMlirModule, outputFileName);
        return 0;
    }

    if (outputFileName.empty())
        outputFileName = "a.out";

    // Create object file
    std::filesystem::path modulePath(getMLIRModuleName(mergedMlirModule));
    const std::string moduleObjectPath = modulePath.replace_extension(".o");
    try {
        exportObjectFile(mergedMlirModule.get(), moduleObjectPath);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error creating object file: " << e.what() << std::endl;
        return 1;
    }

    // Link object file into executable
    try {
        linkObjectFile(moduleObjectPath, outputFileName);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error linking executable: " << e.what() << std::endl;
        return 1;
    }

    // Remove object file unless requested to keep
    if (!upToLower)
        if (std::remove(moduleObjectPath.c_str()) != 0) {
            std::cerr << "Error deleting object file: " << moduleObjectPath << std::endl;
            return 1;
        }

    return 0;
}
