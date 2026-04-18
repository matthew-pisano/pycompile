//
// Created by matthew on 4/4/26.
//

#include "pyruntime/objects/py_set.h"

#include <algorithm>
#include <format>
#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_tuple.h"
#include "pyruntime/runtime_errors.h"
#include "pyruntime/runtime_util.h"

PyObj* PySet::add(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("add() takes exactly one argument");
    PySet* selfSet = dynamic_cast<PySet*>(self);
    if (!selfSet)
        throw PyTypeError("Can only add to set types");

    args[0]->incref();
    selfSet->raw.insert(args[0]);
    return PyNone::None;
}

PyObj* PySet::update(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("update() takes exactly one argument");
    PySet* selfSet = dynamic_cast<PySet*>(self);
    if (!selfSet)
        throw PyTypeError("Can only update set types");

    if (const PySet* srcSet = dynamic_cast<const PySet*>(args[0]))
        for (PyObj* v : srcSet->raw) {
            v->incref();
            selfSet->raw.insert(v);
        }
    else if (const PyList* srcList = dynamic_cast<const PyList*>(args[0]))
        for (PyObj* v : srcList->data()) {
            v->incref();
            selfSet->raw.insert(v);
        }
    else if (const PyTuple* srcTuple = dynamic_cast<const PyTuple*>(args[0]))
        for (PyObj* v : srcTuple->data()) {
            v->incref();
            selfSet->raw.insert(v);
        }
    else
        throw PyTypeError("Can only update with iterable types, got" + args[0]->typeName());

    return PyNone::None;
}

PyObj* PySet::getAttr(const std::string& name) {
    if (name == "update")
        return new PyMethod("update", this, update);
    if (name == "add")
        return new PyMethod("add", this, add);
    throw PyAttributeError(std::format("'{}' object has no attribute '{}'", typeName(), name));
}

PyInt* PySet::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PySet::contains(const PyObj* obj) const {
    return raw.contains(const_cast<PyObj*>(obj)) ? PyBool::True : PyBool::False;
}

size_t PySet::hash() const { throw PyTypeError("Unhashable type " + typeName()); }

std::string PySet::toString() const {
    if (raw.empty())
        return "set()";

    std::string result = "{";
    for (const PyObj* obj : raw)
        result += valueToString(obj, true) + ", ";
    // Remove ', ' from end
    result.pop_back();
    result.pop_back();
    result += "}";
    return result;
}

std::string PySet::typeName() const { return "set"; }

bool PySet::isTruthy() const { return !raw.empty(); }

PySetData PySet::data() const { return raw; }

PySetData& PySet::data() { return raw; }

std::partial_ordering PySet::operator<=>(const PyObj& other) const noexcept {
    if (const PySet* s = dynamic_cast<const PySet*>(&other)) {
        if (raw.size() != s->raw.size())
            return raw.size() <=> s->raw.size();

        for (PyObj* elem : raw)
            if (!s->raw.contains(elem))
                return std::partial_ordering::unordered;
        return std::partial_ordering::equivalent;
    }
    return std::partial_ordering::unordered;
}

bool PySet::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}

PyObj* PySetIter::next(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw PyTypeError("next() takes no arguments");
    PySetIter* selfIter = dynamic_cast<PySetIter*>(self);
    if (!selfIter)
        throw PyTypeError("Can only get the next value of iterator types");
    if (selfIter->it == selfIter->set.end())
        throw PyStopIteration();

    PyObj* obj = *selfIter->it;
    obj->incref();
    ++selfIter->it;
    return obj;
}

std::partial_ordering PySetIter::operator<=>(const PyObj& other) const noexcept {
    if (const PySetIter* iter = dynamic_cast<const PySetIter*>(&other)) {
        if (it == iter->it)
            return std::partial_ordering::equivalent;
    }
    return std::partial_ordering::unordered;
}
