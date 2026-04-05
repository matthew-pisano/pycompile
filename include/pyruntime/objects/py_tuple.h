//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_TUPLE_H
#define PYCOMPILE_PY_TUPLE_H
#include <utility>
#include <vector>

#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PyTupleData = std::vector<PyObj*>;

struct PyTuple : PyObj {
    explicit PyTuple(PyTupleData tuple) : raw(std::move(tuple)) {}

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PyTupleData data() const;

    bool operator==(const PyObj& other) const override;

private:
    PyTupleData raw;
};

#endif // PYCOMPILE_PY_TUPLE_H
