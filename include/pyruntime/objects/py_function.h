//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_FUNCTION_H
#define PYCOMPILE_PY_FUNCTION_H
#include "py_object.h"

using PyFunctionType = PyObj* (*) (PyObj**, int64_t);

struct PyFunction : PyObj {
    explicit PyFunction(const std::string& fnName, const PyFunctionType& func) : fnName(fnName), fn(func) {}

    std::string toString() const override;

    std::string typeName() const override;

    bool isTruthy() const override;

    const std::unordered_map<std::string, PyMethod> attrs() const override;

    std::string funcName() const;

    PyFunctionType data() const;

private:
    std::string fnName;
    PyFunctionType fn;
};

#endif // PYCOMPILE_PY_FUNCTION_H
