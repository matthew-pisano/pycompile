//
// Created by matthew on 3/2/26.
//

#include "bytecode/pythoncode.h"

#include <stdexcept>

#include "bytecode/python_error.h"


CompiledModule compilePythonSource(const std::string& source, const std::string& filename,
                                   const std::string& moduleName) {
    PyObject* code = Py_CompileString(source.c_str(), filename.c_str(), Py_file_input);
    if (!code) {
        PyObject* type;
        PyObject* value;
        PyObject* traceback;
        PyErr_Fetch(&type, &value, &traceback); // Get Python error
        PyErr_NormalizeException(&type, &value, &traceback);

        std::string pythonError = "Unknown error";
        if (value)
            if (PyObject* pyStr = PyObject_Str(value)) {
                pythonError = PyUnicode_AsUTF8(pyStr);
                Py_DECREF(pyStr);
            }

        Py_XDECREF(type);
        Py_XDECREF(value);
        Py_XDECREF(traceback);

        throw std::runtime_error(pythonError);
    }
    return {filename, moduleName, code}; // code is an owned reference
}
