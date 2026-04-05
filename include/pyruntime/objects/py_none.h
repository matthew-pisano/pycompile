//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_NONE_H
#define PYCOMPILE_PY_NONE_H
#include "py_method.h"
#include "py_object.h"

struct PyNone : PyObj {
    explicit PyNone() {}

    size_t hash() const override;

    std::string toString() const override;

    std::string typeName() const override;

    bool isTruthy() const override;

    bool operator==(const PyObj& other) const override;
};

#endif // PYCOMPILE_PY_NONE_H
