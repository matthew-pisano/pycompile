//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_tuple.h"

#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/runtime_util.h"

PyInt* PyTuple::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PyTuple::contains(const PyObj* obj) const {
    for (const PyObj* elem : raw)
        if (*elem == *obj)
            return new PyBool(true);
    return new PyBool(false);
}

PyObj* PyTuple::idx(const PyObj* idx) const {
    if (const PyInt* idxVal = dynamic_cast<const PyInt*>(idx)) {
        int64_t index = idxVal->data();
        if (index < 0)
            index += static_cast<int64_t>(raw.size());
        if (index < 0 || index >= static_cast<int64_t>(raw.size()))
            throw std::runtime_error("tuple index out of range");
        raw[index]->incref(); // Return a new reference to the indexed value
        return raw[index];
    }
    throw std::runtime_error("tuple indices must be integers");
}

size_t PyTuple::hash() const {
    size_t hash = 0;
    for (const PyObj* obj : raw)
        hash ^= obj->hash();
    return hash;
}

std::string PyTuple::toString() const {
    if (raw.empty())
        return "()";

    std::string result = "(";
    for (const PyObj* elem : raw)
        result += valueToString(elem, true) + ", ";
    result.pop_back();
    // Only remove last comma if more than a 1-tuple
    if (raw.size() > 1)
        result.pop_back();
    result += ")";
    return result;
}

std::string PyTuple::typeName() const { return "tuple"; }

bool PyTuple::isTruthy() const { return !raw.empty(); }

PyListData PyTuple::data() const { return raw; }

std::partial_ordering PyTuple::operator<=>(const PyObj& other) const noexcept {
    if (const PyTuple* t = dynamic_cast<const PyTuple*>(&other)) {
        if (raw.size() != t->raw.size())
            return raw.size() <=> t->raw.size();

        for (size_t i = 0; i < raw.size(); i++)
            if (*raw[i] != *t->raw[i])
                return std::partial_ordering::unordered;
        return std::partial_ordering::equivalent;
    }
    return std::partial_ordering::unordered;
}

bool PyTuple::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}

PyObj* PyTupleIter::next(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw std::runtime_error("next() takes no arguments");
    PyTupleIter* selfIter = dynamic_cast<PyTupleIter*>(self);
    if (!selfIter)
        throw std::runtime_error("Can only get the next value of iterator types");
    if (selfIter->it == selfIter->tuple.end())
        throw std::runtime_error("StopIteration()");

    PyObj* obj = *selfIter->it;
    obj->incref();
    ++selfIter->it;
    return obj;
}

std::partial_ordering PyTupleIter::operator<=>(const PyObj& other) const noexcept {
    if (const PyTupleIter* iter = dynamic_cast<const PyTupleIter*>(&other))
        return it <=> iter->it;
    return std::partial_ordering::unordered;
}
