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

    std::string toString() const override;

    std::string typeName() const override;

    bool isTruthy() const override;

    const std::unordered_map<std::string, PyMethod> attrs() override { return {}; }

    double_t data() const;

    bool operator==(const PyObj& other) const override;

private:
    double_t raw;
};

#endif // PYCOMPILE_PY_FLOAT_H
