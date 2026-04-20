//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_TUPLE_H
#define PYCOMPILE_PY_TUPLE_H
#include <utility>
#include <vector>

#include "py_iter.h"
#include "py_method.h"
#include "py_object.h"


struct PyNone;

using PyTupleData = std::vector<PyObj*>;

struct PyTuple : PyObj {
    explicit PyTuple(PyTupleData tuple) : raw(std::move(tuple)) {}

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] PyTupleData data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PyTupleData raw;
    
    friend struct PyTupleIter;
};


struct PyTupleIter : PyIter {
    explicit PyTupleIter(PyTuple* tuple) : tuple(tuple) {
        this->tuple->incref();
        it = this->tuple->raw.begin();
        end = this->tuple->raw.end();
    }
    ~PyTupleIter() override;

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
