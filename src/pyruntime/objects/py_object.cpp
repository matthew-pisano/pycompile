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

PyMethod* PyObj::getAttr(const char* attr) {
    const auto it = attrs().find(attr);
    if (it == attrs().end())
        throw std::runtime_error(std::format("{} has no attribute '{}'", typeName(), attr));
    return new PyMethod{attr, this, it->second.data()};
}

PyInt* PyObj::len() const { throw std::runtime_error(std::format("object of type '{}' has no len()", typeName())); }

PyStr* PyObj::str() const { return new PyStr(toString()); }
