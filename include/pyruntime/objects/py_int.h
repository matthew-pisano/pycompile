//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_INT_H
#define PYCOMPILE_PY_INT_H
#include "py_method.h"
#include "py_object.h"

struct PyInt : PyObj {
    explicit PyInt(const int64_t integer) : raw(integer) {}

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] int64_t data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    int64_t raw;
};

#endif // PYCOMPILE_PY_INT_H
