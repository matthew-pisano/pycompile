// RAII wrapper for Python interpreter initialization/finalization
// Ensures the Python interpreter is finalized after all local objects that may hold PyObject* are destroyed.

#ifndef PYCOMPILE_PYTHON_RAII_H
#define PYCOMPILE_PYTHON_RAII_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>


class PythonInterpreter {
public:
    PythonInterpreter() {
        if (!Py_IsInitialized())
            Py_Initialize();
    }

    ~PythonInterpreter() {
        if (Py_IsInitialized())
            Py_Finalize();
    }

    // Non-copyable/non-movable to avoid accidental lifetime issues
    PythonInterpreter(const PythonInterpreter&) = delete;

    PythonInterpreter& operator=(const PythonInterpreter&) = delete;

    PythonInterpreter(PythonInterpreter&&) = delete;

    PythonInterpreter& operator=(PythonInterpreter&&) = delete;
};

#endif // PYCOMPILE_PYTHON_RAII_H

