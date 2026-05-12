//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_FUNCTION_H
#define PYCOMPILE_PY_FUNCTION_H
#include <utility>

#include "pytypes/py_object.hpp"

using PyFunctionData = PyObj* (*) (PyObj**, int64_t);

/**
 * PyFunction represents the function type in Python. It is a callable object that encapsulates a C++ function pointer.
 */
struct PyFunction : PyObj {
    explicit PyFunction(std::string fnName, const PyFunctionData& func) : fnName(std::move(fnName)), fn(func) {}

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns the name of this PyFunction.
    [[nodiscard]] std::string funcName() const;

    /// Returns the raw function pointer of this PyFunction.
    [[nodiscard]] PyFunctionData data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    std::string fnName;
    PyFunctionData fn;
};

#endif // PYCOMPILE_PY_FUNCTION_H
