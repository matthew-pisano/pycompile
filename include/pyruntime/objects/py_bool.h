//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_BOOL_H
#define PYCOMPILE_PY_BOOL_H

#include "py_object.h"

struct PyBool : PyObj {
    explicit PyBool(const bool boolean) : raw(boolean) {}

    std::string toString() const override;

    std::string typeName() const override;

    const std::unordered_map<std::string, PyBoundMethod> attrs() const override { return {}; }

    bool data() const;

    bool operator==(const PyBool& other) const;

private:
    bool raw;
};

#endif // PYCOMPILE_PY_BOOL_H
