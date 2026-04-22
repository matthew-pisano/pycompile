//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_object.h"

#include <format>
#include <iostream>
#include <stdexcept>

#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_errors.h"

void PyObj::incref() { refcount.fetch_add(1, std::memory_order_relaxed); }

bool PyObj::decref() {
    if (refcount == 0)
        throw std::runtime_error("Invalid decref");
    if (refcount.fetch_sub(1, std::memory_order_acq_rel) <= 1) {
        delete this;
        return true;
    }
    return false;
}

PyStr* PyObj::name() const { return new PyStr(typeName()); }

PyObj* PyObj::getAttr(const std::string& name) {
    const std::string selfTypeName = typeName();
    (void) decref();
    throw PyAttributeError(std::format("'{}' object has no attribute '{}'", selfTypeName, name));
}

PyInt* PyObj::len() const { throw PyTypeError(std::format("object of type '{}' has no len()", typeName())); }

PyStr* PyObj::str() const { return new PyStr(toString()); }

PyBool* PyObj::contains(const PyObj*) const {
    throw PyTypeError(std::format("Object of type '{}' is not iterable", typeName()));
}

PyObj* PyObj::idx(const PyObj*) const {
    throw PyTypeError(std::format("Object of type '{}' is not subscriptable", typeName()));
}

void PyObj::setIdx(PyObj*, PyObj*) {
    throw PyTypeError(std::format("Object of type '{}' does not support item assignment", typeName()));
}
