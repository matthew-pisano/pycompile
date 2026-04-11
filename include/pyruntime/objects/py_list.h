//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_LIST_H
#define PYCOMPILE_PY_LIST_H
#include <utility>
#include <vector>

#include "py_iter.h"
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

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    void setIdx(const PyObj* idx, PyObj* value) override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PyListData data() const;

    PyListData& data();

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PyListData raw;
};


struct PyListIter : PyIter {
    explicit PyListIter(PyListData list) : list(std::move(list)) { it = this->list.begin(); }

    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<list_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "list_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PyListData::iterator it;
    PyListData list;
};

#endif // PYCOMPILE_PY_LIST_H
