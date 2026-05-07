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

/**
 * PySet represents the set type in Python. It is a mutable unordered collection of unique elements. Elements must be
 * hashable.
 */
struct PySet : PyObj {
    explicit PySet(PySetData set) : raw(std::move(set)) {}
    ~PySet() override;

    /// Adds an element to the set. If the element is already present, this method does nothing.
    static PyObj* add(PyObj* self, PyObj** args, int64_t argc);

    /// Updates the set, adding elements from another iterable. If an element is already present, it is not added again.
    static PyObj* update(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns a copy of the raw set data.
    [[nodiscard]] PySetData data() const;

    /// Returns a mutable reference to the raw set data.
    PySetData& data();

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PySetData raw;

    friend struct PySetIter;
};


/**
 * PySetIter represents an iterator over the elements of a PySet.
 */
struct PySetIter : PyIter {
    explicit PySetIter(PySet* set) : set(set) {
        this->set->incref();
        it = this->set->raw.begin();
        end = this->set->raw.end();
    }
    ~PySetIter() override;

    /// Returns the next element in the set, or raises StopIteration if there are no more elements.
    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<set_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "set_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PySetData::iterator it;
    PySetData::iterator end;
    PySet* set;
};

#endif // PYCOMPILE_PY_SET_H
