//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_FUNCTION_H
#define PYCOMPILE_PY_FUNCTION_H
#include <utility>

#include "py_method.h"
#include "py_object.h"

using PyFunctionData = PyObj* (*) (PyObj**, int64_t);

struct PyFunction : PyObj {
    explicit PyFunction(std::string fnName, const PyFunctionData& func) : fnName(std::move(fnName)), fn(func) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] std::string funcName() const;

    [[nodiscard]] PyFunctionData data() const;

    bool operator==(const PyObj& other) const override;

private:
    std::string fnName;
    PyFunctionData fn;
};

#endif // PYCOMPILE_PY_FUNCTION_H
