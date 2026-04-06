//
// Created by matthew on 4/4/26.
//

#ifndef PYCOMPILE_PY_DICT_H
#define PYCOMPILE_PY_DICT_H
#include <unordered_map>
#include <utility>

#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PyDictData = std::unordered_map<PyObj*, PyObj*, PyObjPtrHash, PyObjPtrEqual>;

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

    size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PyDictData data() const;

    PyDictData& data();

    bool operator==(const PyObj& other) const override;

private:
    PyDictData raw;
};

#endif // PYCOMPILE_PY_DICT_H
