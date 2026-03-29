//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_METHOD_H
#define PYCOMPILE_PY_METHOD_H
#include "py_function.h"
#include "py_object.h"

struct PyMethod : PyObj {
    explicit PyMethod(const std::string& fnName, PyObj* self, const PyFunctionType& func) :
        fnName(fnName), self(self), fn(func) {}

    std::string toString() const override;

    std::string typeName() const override;

    bool isTruthy() const override;

    const std::unordered_map<std::string, PyMethod> attrs() const override;

    PyFunctionType data() const;

private:
    std::string fnName;
    PyObj* self;
    PyFunctionType fn;
};

#endif // PYCOMPILE_PY_METHOD_H
