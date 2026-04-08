//
// Created by matthew on 4/4/26.
//

#ifndef PYCOMPILE_PY_SET_H
#define PYCOMPILE_PY_SET_H
#include <unordered_set>
#include <utility>

#include "py_iter.h"
#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PySetData = std::unordered_set<PyObj*, PyObjPtrHash, PyObjPtrEqual>;

struct PySet : PyObj {
    explicit PySet(PySetData set) : raw(std::move(set)) {}

    static PyObj* add(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* update(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PySetData data() const;

    PySetData& data();

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PySetData raw;
};


struct PySetIter : PyIter {
    explicit PySetIter(const PySetData::iterator begin, const PySetData::iterator end) : begin(begin), end(end) {}

    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PySetData::iterator begin;
    PySetData::iterator end;
};

#endif // PYCOMPILE_PY_SET_H
