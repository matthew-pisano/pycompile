//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_STR_H
#define PYCOMPILE_PY_STR_H
#include "py_object.h"

struct PyStr : PyObj {
    explicit PyStr(const std::string& str) : raw(str) {}

    PyInt* len() const override;

    std::string toString() const override;

    std::string typeName() const override;

    const std::unordered_map<std::string, PyMethod> attrs() const override;

    std::string data();

    bool operator==(const PyStr& other) const;

private:
    std::string raw;
};

#endif // PYCOMPILE_PY_STR_H
