//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_object.h"

#include <format>
#include <stdexcept>

#include "pyruntime/objects/py_str.h"

void PyObj::incref() { refcount.fetch_add(1, std::memory_order_relaxed); }

void PyObj::decref() {
    if (refcount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        delete this;
}

PyStr* PyObj::name() const { return new PyStr(typeName()); }

PyObj* PyObj::getAttr(const std::string& name) { throw std::runtime_error("object has no attribute '" + name + "'"); }

PyInt* PyObj::len() const { throw std::runtime_error(std::format("object of type '{}' has no len()", typeName())); }

PyStr* PyObj::str() const { return new PyStr(toString()); }

PyBool* PyObj::contains(const PyObj*) const {
    throw std::runtime_error(std::format("Object of type '{}' is not iterable", typeName()));
}

PyObj* PyObj::idx(const PyObj*) const {
    throw std::runtime_error(std::format("Object of type '{}' is not iterable", typeName()));
}

void PyObj::setIdx(const PyObj*, PyObj*) {
    throw std::runtime_error(std::format("Object of type '{}' does not support item assignment", typeName()));
}
