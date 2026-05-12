//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_FLOAT_H
#define PYCOMPILE_PY_FLOAT_H
#include <cmath>

#include "pytypes/py_object.h"

/**
 * PyFloat represents the float type in Python. It represents a double-precision floating point number.
 */
struct PyFloat : PyObj {
    explicit PyFloat(const double_t decimal) : raw(decimal) {}

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns the raw double value of this PyFloat.
    [[nodiscard]] double_t data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    double_t raw;
};

#endif // PYCOMPILE_PY_FLOAT_H
