//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_BOOL_H
#define PYCOMPILE_PY_BOOL_H

#include "py_method.h"
#include "py_object.h"

/**
 * PyBool represents the bool type in Python. It is a doubleton type, with only two instances: True and False.
 */
struct PyBool : PyObj {
    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns the raw boolean value of this PyBool.
    [[nodiscard]] bool data() const;

    void incref() override;

    [[nodiscard]] bool decref() override;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

    /// The PyBool truth singleton.
    static PyBool* True;

    /// The PyBool false singleton.
    static PyBool* False;

private:
    bool raw;

    explicit PyBool(const bool boolean) : raw(boolean) {}
};

#endif // PYCOMPILE_PY_BOOL_H
