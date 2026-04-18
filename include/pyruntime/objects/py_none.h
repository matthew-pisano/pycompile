//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_NONE_H
#define PYCOMPILE_PY_NONE_H
#include "py_method.h"
#include "py_object.h"

struct PyNone : PyObj {
    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    void incref() override;

    [[nodiscard]] bool decref() override;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

    static PyNone* None;

private:
    PyNone() = default;
};

#endif // PYCOMPILE_PY_NONE_H
