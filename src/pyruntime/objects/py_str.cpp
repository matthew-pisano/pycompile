//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_str.h"

#include "pyruntime/objects/py_int.h"

PyInt* PyStr::len() const { return new PyInt(raw.size()); }

std::string PyStr::toString() const { return raw; }

std::string PyStr::typeName() const { return "str"; }

const std::unordered_map<std::string, PyMethod> PyStr::attrs() const { return {}; }

std::string PyStr::data() { return raw; }

bool PyStr::operator==(const PyStr& other) const {
    return raw == other.raw;
}
