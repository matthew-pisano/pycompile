//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_INT_H
#define PYCOMPILE_PY_INT_H
#include "py_method.h"
#include "py_object.h"

struct PyInt : PyObj {
    explicit PyInt(const int64_t integer) : raw(integer) {}

    std::string toString() const override;

    std::string typeName() const override;

    bool isTruthy() const override;

    const std::unordered_map<std::string, PyMethod> attrs() const override { return {}; }

    int64_t data() const;

    bool operator==(const PyObj& other) const override;

private:
    int64_t raw;
};

#endif // PYCOMPILE_PY_INT_H
