//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_list.h"

#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_tuple.h"
#include "pyruntime/runtime_util.h"

PyObj* PyList::append(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("append() takes exactly one argument");
    PyList* selfList = dynamic_cast<PyList*>(self);
    if (!selfList)
        throw std::runtime_error("Can only append to list types");

    args[0]->incref();
    selfList->raw.push_back(args[0]);
    return new PyNone();
}

PyObj* PyList::extend(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("extend() takes exactly one argument");
    PyList* selfList = dynamic_cast<PyList*>(self);
    if (!selfList)
        throw std::runtime_error("Can only extend list types");

    if (const PyList* srcList = dynamic_cast<const PyList*>(args[0]))
        for (PyObj* v : srcList->raw) {
            v->incref();
            selfList->raw.push_back(v);
        }
    else if (const PySet* srcSet = dynamic_cast<const PySet*>(args[0]))
        for (PyObj* v : srcSet->data()) {
            v->incref();
            selfList->raw.push_back(v);
        }
    else if (const PyTuple* srcTuple = dynamic_cast<const PyTuple*>(args[0]))
        for (PyObj* v : srcTuple->data()) {
            v->incref();
            selfList->raw.push_back(v);
        }
    else
        throw std::runtime_error("Can only extend with iterable types, got" + args[0]->typeName());

    return new PyNone();
}

PyObj* PyList::getAttr(const std::string& name) {
    if (name == "append")
        return new PyMethod("append", this, append);
    if (name == "extend")
        return new PyMethod("extend", this, extend);
    throw std::runtime_error(this->typeName() + " has no attribute '" + name + "'");
}

PyInt* PyList::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PyList::contains(const PyObj* obj) const {
    for (const PyObj* elem : raw)
        if (*elem == *obj)
            return new PyBool(true);
    return new PyBool(false);
}

PyObj* PyList::idx(const PyObj* idx) const {
    if (const PyInt* idxVal = dynamic_cast<const PyInt*>(idx)) {
        int64_t index = idxVal->data();
        if (index < 0)
            index += static_cast<int64_t>(raw.size());
        if (index < 0 || index >= static_cast<int64_t>(raw.size()))
            throw std::runtime_error("list index out of range");
        raw[index]->incref(); // Return a new reference to the indexed value
        return raw[index];
    }
    throw std::runtime_error("list indices must be integers");
}

void PyList::setIdx(const PyObj* idx, PyObj* value) {
    if (const PyInt* idxVal = dynamic_cast<const PyInt*>(idx)) {
        int64_t index = idxVal->data();
        if (index < 0)
            index += static_cast<int64_t>(raw.size());
        if (index < 0 || index >= static_cast<int64_t>(raw.size()))
            throw std::runtime_error("list index out of range");
        raw[index]->decref(); // Decref the old value
        raw[index] = value;
        return;
    }
    throw std::runtime_error("list indices must be integers");
}

size_t PyList::hash() const { throw std::runtime_error("Unhashable type " + typeName()); }

std::string PyList::toString() const {
    if (raw.empty())
        return "[]";

    std::string result = "[";
    for (size_t i = 0; i < raw.size() - 1; i++)
        result += valueToString(raw[i], true) + ", ";
    result += valueToString(raw[raw.size() - 1], true) + "]";
    return result;
}

std::string PyList::typeName() const { return "list"; }

bool PyList::isTruthy() const { return !raw.empty(); }

PyListData PyList::data() const { return raw; }

PyListData& PyList::data() { return raw; }

std::partial_ordering PyList::operator<=>(const PyObj& other) const noexcept {
    if (const PyList* l = dynamic_cast<const PyList*>(&other)) {
        if (raw.size() != l->raw.size())
            return raw.size() <=> l->raw.size();

        for (size_t i = 0; i < raw.size(); i++)
            if (*raw[i] != *l->raw[i])
                return std::partial_ordering::unordered;
        return std::partial_ordering::equivalent;
    }
    return std::partial_ordering::unordered;
}

bool PyList::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}

PyObj* PyListIter::next(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw std::runtime_error("next() takes no arguments");
    PyListIter* selfIter = dynamic_cast<PyListIter*>(self);
    if (!selfIter)
        throw std::runtime_error("Can only get the next value of iterator types");
    if (selfIter->it == selfIter->list.end())
        throw std::runtime_error("StopIteration()");

    PyObj* obj = *selfIter->it;
    obj->incref();
    ++selfIter->it;
    return obj;
}

std::partial_ordering PyListIter::operator<=>(const PyObj& other) const noexcept {
    if (const PyListIter* iter = dynamic_cast<const PyListIter*>(&other))
        return it <=> iter->it;
    return std::partial_ordering::unordered;
}
