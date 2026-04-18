//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_object.h"

#include <format>
#include <iostream>
#include <stdexcept>

#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_errors.h"

void PyObj::incref() {
    const void* addr = static_cast<void*>(this);
    std::cerr << std::format("======== Incref {} '{}' ({}) to: {}", typeName(), toString(), addr, refcount + 1)
              << std::endl;
    refcount.fetch_add(1, std::memory_order_relaxed);
}

bool PyObj::decref() {
    const void* addr = static_cast<void*>(this);
    std::cerr << std::format("======== Decref {} '{}' ({}) to: {}", typeName(), toString(), addr, refcount - 1)
              << std::endl;
    if (refcount.fetch_sub(1, std::memory_order_acq_rel) <= 1) {
        delete this;
        return true;
    }
    return false;
}

PyStr* PyObj::name() const { return new PyStr(typeName()); }

PyObj* PyObj::getAttr(const std::string& name) {
    throw PyAttributeError(std::format("'{}' object has no attribute '{}'", typeName(), name));
}

PyInt* PyObj::len() const { throw PyTypeError(std::format("object of type '{}' has no len()", typeName())); }

PyStr* PyObj::str() const { return new PyStr(toString()); }

PyBool* PyObj::contains(const PyObj*) const {
    throw PyTypeError(std::format("Object of type '{}' is not iterable", typeName()));
}

PyObj* PyObj::idx(const PyObj*) const {
    throw PyTypeError(std::format("Object of type '{}' is not subscriptable", typeName()));
}

void PyObj::setIdx(const PyObj*, PyObj*) {
    throw PyTypeError(std::format("Object of type '{}' does not support item assignment", typeName()));
}
