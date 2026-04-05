//
// Created by matthew on 4/4/26.
//

#ifndef PYCOMPILE_PY_SET_H
#define PYCOMPILE_PY_SET_H
#include <unordered_set>
#include <utility>

#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PySetData = std::unordered_set<PyObj*>;

struct PySet : PyObj {
    explicit PySet(PySetData set) : raw(std::move(set)) {}

    static PyObj* add(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* update(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PySetData data() const;

    PySetData& data();

    bool operator==(const PyObj& other) const override;

private:
    PySetData raw;
};

#endif // PYCOMPILE_PY_SET_H
