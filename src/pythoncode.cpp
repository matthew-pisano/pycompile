//
// Created by matthew on 3/2/26.
//

#include "pythoncode.h"

#include <stdexcept>

#include "utils.h"


CompiledModule compilePythonSource(const std::string& source, const std::string& filename,
                                   const std::string& moduleName) {
    PyObject* code = Py_CompileString(source.c_str(), filename.c_str(), Py_file_input);
    if (!code) {
        const std::string err = getPythonErrorTraceback();
        throw std::runtime_error("Error during compilation: " + err);
    }
    return {filename, moduleName, code}; // code is an owned reference
}
