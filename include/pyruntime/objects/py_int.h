//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_INT_H
#define PYCOMPILE_PY_INT_H
#include "py_object.h"

struct PyInt : PyObj {
    explicit PyInt(const int64_t integer) : raw(integer) {}

    std::string toString() const override;

    std::string typeName() const override;

    const std::unordered_map<std::string, PyBoundMethod> attrs() const override { return {}; }

    int64_t data() const;

    bool operator==(const PyInt& other) const;

private:
    int64_t raw;
};

#endif // PYCOMPILE_PY_INT_H
