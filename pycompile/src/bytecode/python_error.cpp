//
// Created by matthew on 3/5/26.
//

#include "bytecode/python_error.h"

#include <Python.h>
#include <stdexcept>


std::string getPythonErrorTraceback() {
    if (!PyErr_Occurred())
        return "Unknown Python error";

    PyObject* pType;
    PyObject* pValue;
    PyObject* pTraceback;
    PyErr_Fetch(&pType, &pValue, &pTraceback);
    PyErr_NormalizeException(&pType, &pValue, &pTraceback);

    std::string result;

    // Get the string representation of the exception value
    if (pValue) {
        if (PyObject* pStr = PyObject_Str(pValue)) {
            result = PyUnicode_AsUTF8(pStr);
            Py_DECREF(pStr);
        }
    }

    Py_XDECREF(pType);
    Py_XDECREF(pValue);
    Py_XDECREF(pTraceback);

    return result.empty() ? "Unknown Python error" : result;
}
