//
// Created by matthew on 4/4/26.
//

#include "pyruntime/objects/py_set.h"

#include <algorithm>
#include <stdexcept>

#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/runtime_util.h"

PyObj* PySet::add(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("add() takes exactly one argument");
    PySet* selfSet = dynamic_cast<PySet*>(self);
    if (!selfSet)
        throw std::runtime_error("Can only add to set types");

    args[0]->incref();
    selfSet->raw.insert(args[0]);
    return new PyNone();
}

PyObj* PySet::update(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("update() takes exactly one argument");
    PySet* selfSet = dynamic_cast<PySet*>(self);
    if (!selfSet)
        throw std::runtime_error("Can only update set types");

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
    return new PyNone();
}

PyObj* PySet::getAttr(const std::string& name) {
    if (name == "update")
        return new PyMethod("update", this, update);
    if (name == "add")
        return new PyMethod("add", this, add);
    throw std::runtime_error(this->typeName() + " has no attribute '" + name + "'");
}

PyInt* PySet::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

size_t PySet::hash() const { throw std::runtime_error("Unhashable type " + typeName()); }

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

bool PySet::operator==(const PyObj& other) const {
    if (const PySet* s = dynamic_cast<const PySet*>(&other)) {
        if (raw.size() != s->raw.size())
            return false;
        return std::ranges::all_of(raw, [s](PyObj* elem) { return s->raw.contains(elem); });
    }
    return false;
}
