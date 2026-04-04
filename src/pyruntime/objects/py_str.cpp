//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_str.h"

#include "pyruntime/objects/py_int.h"

PyInt* PyStr::len() const { return new PyInt(raw.size()); }

std::string PyStr::toString() const { return raw; }

std::string PyStr::typeName() const { return "str"; }

bool PyStr::isTruthy() const { return !raw.empty(); }

const std::unordered_map<std::string, PyMethod> PyStr::attrs() { return {}; }

std::string PyStr::data() const { return raw; }

bool PyStr::operator==(const PyObj& other) const {
    if (const PyStr* s = dynamic_cast<const PyStr*>(&other))
        return raw == s->data();
    return false;
}
