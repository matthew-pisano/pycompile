//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_BOOL_H
#define PYCOMPILE_PY_BOOL_H

#include "py_method.h"
#include "py_object.h"

struct PyBool : PyObj {
    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] bool data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

    static PyBool* True;
    static PyBool* False;

private:
    bool raw;

    explicit PyBool(const bool boolean) : raw(boolean) {}
};

#endif // PYCOMPILE_PY_BOOL_H
