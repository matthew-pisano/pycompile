//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_LIST_H
#define PYCOMPILE_PY_LIST_H
#include <utility>
#include <vector>

#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PyListData = std::vector<PyObj*>;

struct PyList : PyObj {
    explicit PyList(PyListData list) : raw(std::move(list)) {}

    static PyObj* append(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* extend(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PyListData data() const;

    PyListData& data();

    bool operator==(const PyObj& other) const override;

private:
    PyListData raw;
};

#endif // PYCOMPILE_PY_LIST_H
