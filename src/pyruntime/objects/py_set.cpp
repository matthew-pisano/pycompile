//
// Created by matthew on 4/4/26.
//

#include "pyruntime/objects/py_set.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/runtime_util.h"

PyObj* PySet::add(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("add() takes exactly one argument");
    pyir_setAdd(self, args[0]);
    return new PyNone();
}

PyObj* PySet::update(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("update() takes exactly one argument");
    pyir_setUpdate(self, args[0]);
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

std::unordered_set<PyObj*> PySet::data() const { return raw; }

std::unordered_set<PyObj*>& PySet::data() { return raw; }

bool PySet::operator==(const PyObj& other) const {
    if (const PySet* s = dynamic_cast<const PySet*>(&other))
        return raw == s->data();
    return false;
}
