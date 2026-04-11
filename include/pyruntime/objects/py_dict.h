//
// Created by matthew on 4/4/26.
//

#ifndef PYCOMPILE_PY_DICT_H
#define PYCOMPILE_PY_DICT_H
#include <map>
#include <utility>

#include "py_iter.h"
#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PyDictData = std::map<PyObj*, PyObj*, PyObjPtrCompare>;

struct PyDict : PyObj {
    explicit PyDict(PyDictData dict) : raw(std::move(dict)) {}

    static PyObj* get(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* keys(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* values(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* items(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* update(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    void setIdx(const PyObj* idx, PyObj* value) override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PyDictData data() const;

    PyDictData& data();

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PyDictData raw;
};


struct PyDictIter : PyIter {
    explicit PyDictIter(PyDictData dict) : dict(std::move(dict)) { it = this->dict.begin(); }

    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<dict_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "dict_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PyDictData::iterator it;
    PyDictData dict;
};

#endif // PYCOMPILE_PY_DICT_H
