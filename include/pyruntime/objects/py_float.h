//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_FLOAT_H
#define PYCOMPILE_PY_FLOAT_H
#include <cmath>

#include "py_method.h"
#include "py_object.h"

struct PyFloat : PyObj {
    explicit PyFloat(const double_t decimal) : raw(decimal) {}

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] double_t data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    double_t raw;
};

#endif // PYCOMPILE_PY_FLOAT_H
