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
 * Prints Python Bytecode modules in textual format
 * @param bytecodeModules The bytecode modules to print
 */
void printByteCode(const std::vector<ByteCodeModule>& bytecodeModules) {
    for (const ByteCodeModule& bytecodeModule : bytecodeModules) {
        std::cout << std::format("Bytecode for module '{}' (from file '{}'):\n", bytecodeModule.moduleName,
                                 bytecodeModule.filename) << std::endl;
        printByteCodeModule(bytecodeModule);
        std::cout << std::endl;
    }
}


/**
 * Prints MLIR in textual format
 * @param mlirModule The MLIR module to print
 */
void printMLIR(mlir::ModuleOp mlirModule) {
    mlir::Location loc = mlirModule.getLoc();
    std::string moduleName = "<unknown>";
    if (const mlir::FileLineColLoc fileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(loc))
        moduleName = fileLoc.getFilename().str();

    std::cout << std::format("MLIR for module '{}':\n", moduleName) << std::endl;
    pyir::printMLIRModule(mlirModule);
    std::cout << std::endl;
}


/**
 * Prints lowered LLVM dialect in textual format
 * @param mlirModule The MLIR module to print
 */
void printLLVMDialect(mlir::ModuleOp mlirModule) {
    mlir::Location loc = mlirModule.getLoc();
    std::string moduleName = "<unknown>";
    if (const mlir::FileLineColLoc fileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(loc))
        moduleName = fileLoc.getFilename().str();

    std::cout << std::format("LLVM MLIR dialect for module '{}':\n", moduleName) << std::endl;
    const mlir::OpPrintingFlags flags;
    mlirModule.getOperation()->print(llvm::outs(), flags);
}


/**
 * Prints LLVM IR in textual format
 * @param mlirModule The MLIR module to print
 */
void printLLVMIR(mlir::ModuleOp mlirModule) {
    mlir::Location loc = mlirModule.getLoc();
    std::string moduleName = "<unknown>";
    if (const mlir::FileLineColLoc fileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(loc))
        moduleName = fileLoc.getFilename().str();

    std::cout << std::format("LLVM IR for module '{}':\n", moduleName) << std::endl;
    exportLLVMIR(mlirModule);
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

    PythonInterpreter pyInterp; // Initializes Python via RAII

    // Compile source to Python PyObjects
    std::vector<CompiledModule> compiledModules;
    for (size_t i = 0; i < inputFileNames.size(); i++) {
        const std::string& fileName = inputFileNames[i];
        const std::string& source = fileContents[i];
        try {
            compiledModules.push_back(compilePythonSource(source, fileName, fileName));
        } catch (const std::runtime_error& e) {
            std::cerr << "Error compiling file '" + fileName + "': " + e.what() << std::endl;
            return 1;
        }
    }

    // Disassemble PyObjects into Python bytecode
    std::vector<ByteCodeModule> bytecodeModules;
    for (const CompiledModule& module : compiledModules) {
        try {
            bytecodeModules.push_back(generatePythonBytecode(module));
        } catch (const std::runtime_error& e) {
            std::cerr << "Error generating bytecode for file '" + module.filename + "': " + e.what() << std::endl;
            return 1;
        }
    }
    printByteCode(bytecodeModules);

    // Lower bytecode into PYIR MLIR dialect
    mlir::MLIRContext context; // Must exit scope after all other MLIR instances
    mlir::OwningOpRef<mlir::ModuleOp> mlirModule;
    try {
        // Skip module merging if only one is included
        if (bytecodeModules.size() > 1)
            mlirModule = pyir::generateMLIR(context, bytecodeModules);
        else
            mlirModule = pyir::generateMLIR(context, bytecodeModules[0]);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error generating mlir from modules: " << e.what() << std::endl;
        return 1;
    }
    printMLIR(mlirModule.get());

    // Lower PYIR to an LLVM MLIR dialect
    try {
        lowerToLLVM(context, mlirModule.get());
    } catch (const std::runtime_error& e) {
        std::cerr << "Error lowering mlir module: " << e.what() << std::endl;
        return 1;
    }
    printLLVMDialect(mlirModule.get());
    printLLVMIR(mlirModule.get());

    // Create object file
    try {
        exportObjectFile(mlirModule.get(), "out/a.o");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error creating object file: " << e.what() << std::endl;
        return 1;
    }
    // Link object file into executable
    try {
        linkObjectFile("out/a.o", "out/a.out");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error linking executable: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
