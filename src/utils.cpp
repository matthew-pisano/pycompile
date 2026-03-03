//
// Created by matthew on 3/1/26.
//

#include "utils.h"

#include <Python.h>
#include <stdexcept>
#include <fstream>
#include <sstream>

std::string getPythonErrorTraceback() {
    PyObject* exc = PyErr_GetRaisedException();
    if (!exc)
        throw std::runtime_error("No active Python exception");

    // Import the traceback module to format the exception
    PyObject* tracebackModule = PyImport_ImportModule("traceback");
    std::string msg;

    if (tracebackModule) {
        PyObject* lines = PyObject_CallMethod(tracebackModule, "format_exception",
                                              "OOO",
                                              PyObject_Type(exc), exc,
                                              PyException_GetTraceback(exc));
        if (lines) {
            PyObject* joined = PyUnicode_Join(PyUnicode_FromString(""), lines);
            if (joined)
                msg = PyUnicode_AsUTF8(joined);

            Py_XDECREF(joined);
            Py_DECREF(lines);
        }
        Py_DECREF(tracebackModule);
    } else {
        // If we can't import traceback, fall back to just the exception type and message
        PyObject* str = PyObject_Str(exc);
        if (str)
            msg = PyUnicode_AsUTF8(str);
        Py_XDECREF(str);
    }

    Py_DECREF(exc);
    return msg;
}

std::string readFileString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filename);

    std::ostringstream oss;
    oss << file.rdbuf();
    file.close();

    return oss.str();
}
