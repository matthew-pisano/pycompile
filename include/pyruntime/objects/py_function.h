//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_FUNCTION_H
#define PYCOMPILE_PY_FUNCTION_H
#include <utility>

#include "py_method.h"
#include "py_object.h"

using PyFunctionType = PyObj* (*) (PyObj**, int64_t);

struct PyFunction : PyObj {
    explicit PyFunction(std::string fnName, const PyFunctionType& func) : fnName(std::move(fnName)), fn(func) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] const std::unordered_map<std::string, PyMethod> attrs() override { return {}; }

    [[nodiscard]] std::string funcName() const;

    [[nodiscard]] PyFunctionType data() const;

    bool operator==(const PyObj& other) const override;

private:
    std::string fnName;
    PyFunctionType fn;
};

#endif // PYCOMPILE_PY_FUNCTION_H
