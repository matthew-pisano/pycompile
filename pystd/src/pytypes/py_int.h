//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_INT_H
#define PYCOMPILE_PY_INT_H
#include "pytypes/py_object.h"

/**
 * PyInt represents the int type in Python. It represents a 64-bit signed integer.
 */
struct PyInt : PyObj {
    explicit PyInt(const int64_t integer) : raw(integer) {}

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns the raw integer value of this PyInt.
    [[nodiscard]] int64_t data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    int64_t raw;
};

#endif // PYCOMPILE_PY_INT_H
