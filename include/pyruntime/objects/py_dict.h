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

/**
 * PyDict represents the dict type in Python. It is a mutable mapping type that maps keys to values. Keys must be
 * hashable, and values can be any Python object.
 */
struct PyDict : PyObj {
    explicit PyDict(PyDictData dict) : raw(std::move(dict)) {}
    ~PyDict() override;

    /// Safely gets the value associated with the given key, returning None if the key is not present in the dict.
    static PyObj* get(PyObj* self, PyObj** args, int64_t argc);

    /// Gets a PyTuple of the keys in this dict.
    static PyObj* keys(PyObj* self, PyObj** args, int64_t argc);

    /// Gets a PyTuple of the values in this dict.
    static PyObj* values(PyObj* self, PyObj** args, int64_t argc);

    /// Gets a PyTuple of the key-value pairs in this dict, where each pair is represented as a 2-tuple.
    static PyObj* items(PyObj* self, PyObj** args, int64_t argc);

    /// Updates this dict with the key-value pairs from another dict.
    static PyObj* update(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    void setIdx(PyObj* idx, PyObj* value) override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns a copy of the raw dict data.
    [[nodiscard]] PyDictData data() const;

    /// Returns a mutable reference to the raw dict data.
    PyDictData& data();

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PyDictData raw;

    friend struct PyDictIter;
};


/**
 * PyDictIter represents an iterator over the keys of a PyDict.
 */
struct PyDictIter : PyIter {
    explicit PyDictIter(PyDict* dict) : dict(dict) {
        this->dict->incref();
        it = this->dict->raw.begin();
        end = this->dict->raw.end();
    }
    ~PyDictIter() override;

    /// Returns the next key in the dict, or raises StopIteration if there are no more keys.
    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<dict_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "dict_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PyDictData::iterator it;
    PyDictData::iterator end;
    PyDict* dict;
};

#endif // PYCOMPILE_PY_DICT_H
