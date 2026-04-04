//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_STR_H
#define PYCOMPILE_PY_STR_H
#include "py_method.h"
#include "py_object.h"

struct PyStr : PyObj {
    explicit PyStr(const std::string& str) : raw(str) {}
    explicit PyStr(const char c) : raw(1, c) {}

    PyInt* len() const override;

    std::string toString() const override;

    std::string typeName() const override;

    bool isTruthy() const override;

    std::string data() const;

    bool operator==(const PyObj& other) const override;

private:
    std::string raw;
};

#endif // PYCOMPILE_PY_STR_H
