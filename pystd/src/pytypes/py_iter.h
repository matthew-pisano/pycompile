//
// Created by matthew on 4/7/26.
//

#ifndef PYCOMPILE_PY_ITER_H
#define PYCOMPILE_PY_ITER_H

#include <stdexcept>

#include "pytypes/py_object.h"


struct PyNone;

/**
 * PyIter represents the iterator protocol in Python. It is an abstract base class that defines the interface for all
 * iterable objects in Python.
 */
struct PyIter : PyObj {
    [[nodiscard]] size_t hash() const override { throw std::runtime_error("Unhashable type " + typeName()); }

    [[nodiscard]] bool isTruthy() const override { return true; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override = 0;

    bool operator==(const PyObj& other) const noexcept override {
        return *this <=> other == std::partial_ordering::equivalent;
    }
};

#endif // PYCOMPILE_PY_ITER_H
