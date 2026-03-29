//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_LIST_H
#define PYCOMPILE_PY_LIST_H
#include <vector>

#include "py_object.h"

struct PyList : PyObj {
    PyNone* append(PyObj** args, int64_t argc);

    PyNone* extend(PyObj** args, int64_t argc);

    PyInt* len() const override;

    std::string toString() const override;

    std::string typeName() const override;

    const std::unordered_map<std::string, PyBoundMethod> attrs() const override;

    bool operator==(const PyList& other) const;

private:
    std::vector<PyObj*> raw;
};

#endif // PYCOMPILE_PY_LIST_H
