//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_LIST_H
#define PYCOMPILE_PY_LIST_H
#include <utility>
#include <vector>

#include "pytypes/py_iter.h"
#include "pytypes/py_object.h"


struct PyNone;

using PyListData = std::vector<PyObj*>;

/**
 * A Python list object. This is a mutable sequence type that can hold any number of objects.
 */
struct PyList : PyObj {
    explicit PyList(PyListData list) : raw(std::move(list)) {}
    ~PyList() override;

    /// Append an object to the end of the list.
    static PyObj* append(PyObj* self, PyObj** args, int64_t argc);

    /// Extend the list by appending all the items from the iterable.
    static PyObj* extend(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    void setIdx(PyObj* idx, PyObj* value) override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns a copy of the raw list data.
    [[nodiscard]] PyListData data() const;

    /// Returns a mutable reference to the raw list data.
    PyListData& data();

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    PyListData raw;

    friend struct PyListIter;
};


/**
 * PyListIter represents an iterator over the elements of a PyList.
 */
struct PyListIter : PyIter {
    explicit PyListIter(PyList* list) : list(list) {
        this->list->incref();
        it = this->list->raw.begin();
        end = this->list->raw.end();
    }
    ~PyListIter() override;

    /// Returns the next element in the list, or raises StopIteration if there are no more elements.
    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<list_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "list_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    PyListData::iterator it;
    PyListData::iterator end;
    PyList* list;
};

#endif // PYCOMPILE_PY_LIST_H
