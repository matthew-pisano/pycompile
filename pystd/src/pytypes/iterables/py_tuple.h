//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_TUPLE_H
#define PYCOMPILE_PY_TUPLE_H
#include <utility>
#include <vector>

#include "pytypes/iterables/py_iter.h"
#include "pytypes/py_object.h"


struct PyNone;

using PyTupleData = std::vector<PyObj*>;

/**
 * PyTuple represents the tuple type in Python. It is an immutable sequence type that can hold any number of objects.
 */
struct PyTuple : PyObj {
    explicit PyTuple(PyTupleData tuple) : raw(std::move(tuple)) {}
    ~PyTuple() override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns a copy of the raw tuple data.
    [[nodiscard]] PyTupleData data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PyTupleData raw;

    friend struct PyTupleIter;
};


/**
 * PyTupleIter represents an iterator over the elements of a PyIter.
 */
struct PyTupleIter : PyIter {
    explicit PyTupleIter(PyTuple* tuple) : tuple(tuple) {
        this->tuple->incref();
        it = this->tuple->raw.begin();
        end = this->tuple->raw.end();
    }
    ~PyTupleIter() override;

    /// Returns the next element in the tuple, or raises StopIteration if there are no more elements.
    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<tuple_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "tuple_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PyTupleData::iterator it;
    PyTupleData::iterator end;
    PyTuple* tuple;
};


#endif // PYCOMPILE_PY_TUPLE_H
