#include <CLI/CLI.hpp>
#include <Python.h>
#include <iostream>

#include "bytecode.h"
#include "pythoncode.h"
#include "utils.h"


int main(const int argc, char* argv[]) {
    const std::string name = "pycompile";

    std::vector<std::string> inputFileNames;
    CLI::App app{name + " - Python Compiler", name};
    app.add_option("file", inputFileNames, "A Python file")->required()->allow_extra_args();

    // Set up help message
    app.failure_message([name](const CLI::App* _app, const CLI::Error& e) -> std::string {
        return name + ": error: " + CLI::FailureMessage::simple(_app, e);
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

    Py_Initialize();

    std::vector<CompiledModule> compiledModules;
    for (size_t i = 0; i < inputFileNames.size(); i++) {
        const std::string& fileName = inputFileNames[i];
        const std::string& source = fileContents[i];
        try {
            compiledModules.push_back(compilePythonSource(source, fileName, fileName));
        } catch (const std::runtime_error& e) {
            Py_Finalize();
            std::cerr << "Error compiling file '" + fileName + "': " + e.what() << std::endl;
            return 1;
        }
    }

    std::vector<ByteCodeModule> bytecodeModules;
    for (const CompiledModule& module : compiledModules) {
        try {
            bytecodeModules.push_back(generatePythonBytecode(module));
        } catch (const std::runtime_error& e) {
            Py_Finalize();
            std::cerr << "Error generating bytecode for file '" + module.filename + "': " + e.what() << std::endl;
            return 1;
        }
    }

    for (const ByteCodeModule& bytecodeModule : bytecodeModules) {
        printf("Bytecode for module '%s' (from file '%s'):\n", bytecodeModule.module_name.c_str(),
               bytecodeModule.filename.c_str());
        printByteCodeModule(bytecodeModule);
        printf("\n");
    }

    Py_Finalize();
    return 0;
}
