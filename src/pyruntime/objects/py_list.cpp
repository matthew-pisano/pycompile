//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_list.h"

#include <stdexcept>

#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_set.h"
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
    else
        throw std::runtime_error("Can only extend with iterable types");

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

bool PyList::operator==(const PyObj& other) const {
    if (const PyList* l = dynamic_cast<const PyList*>(&other)) {
        if (raw.size() != l->raw.size())
            return false;

        for (size_t i = 0; i < raw.size(); i++)
            if (*raw[i] != *l->raw[i])
                return false;
        return true;
    }
    return false;
}
