//
// Created by matthew on 3/2/26.
//

#ifndef PYCOMPILE_PYTHONCODE_H
#define PYCOMPILE_PYTHONCODE_H

#include <Python.h>
#include <string>


/**
 * Struct to hold information about a compiled Python module, including the filename, module name, and the compiled
 * code object. The codeObject is an owned reference, and will be decref'd when the CompiledModule is destroyed.
 */
struct CompiledModule {
    std::string filename;
    std::string module_name;
    PyObject* codeObject; // Owned reference to the compiled code object

    CompiledModule(std::string filename, std::string module_name, PyObject* codeObject) :
        filename(std::move(filename)), module_name(std::move(module_name)), codeObject(codeObject) {
    }

    // Move constructor
    CompiledModule(CompiledModule&& other) noexcept :
        filename(std::move(other.filename)), module_name(std::move(other.module_name)), codeObject(other.codeObject) {
        other.codeObject = nullptr; // Prevent double deletion
    }

    ~CompiledModule() {
        if (codeObject)
            Py_DECREF(codeObject);
    }
};


/**
 * Compiles Python source code into a code object, and returns a CompiledModule struct containing the compiled code.
 * @param source The Python source code to compile
 * @param filename The filename to use in the code object (used for error messages and debugging)
 * @param moduleName The name of the module to use in the code object (used for error messages and debugging)
 * @return A CompiledModule struct containing the compiled code object and associated metadata
 * @throws std::runtime_error if there is an error during compilation, with the error traceback in the exception.
 */
CompiledModule compilePythonSource(const std::string& source, const std::string& filename,
                                   const std::string& moduleName);

#endif //PYCOMPILE_PYTHONCODE_H
