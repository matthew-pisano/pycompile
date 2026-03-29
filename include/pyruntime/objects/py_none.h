//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_NONE_H
#define PYCOMPILE_PY_NONE_H
#include "py_object.h"

struct PyNone : PyObj {
    explicit PyNone() {}

    std::string toString() const override;

    std::string typeName() const override;

    const std::unordered_map<std::string, PyBoundMethod> attrs() const override { return {}; }
};

#endif // PYCOMPILE_PY_NONE_H
