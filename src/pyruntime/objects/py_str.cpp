//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_str.h"

#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"

size_t PyStr::hash() const {
    constexpr std::hash<std::string> hasher;
    return hasher(raw);
}

PyInt* PyStr::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PyStr::contains(const PyObj* obj) const {
    if (const PyStr* str = dynamic_cast<const PyStr*>(obj))
        return new PyBool(raw.contains(str->data()));
    throw std::runtime_error("Str objects can only contain other str objects");
}

std::string PyStr::toString() const { return raw; }

std::string PyStr::typeName() const { return "str"; }

bool PyStr::isTruthy() const { return !raw.empty(); }

std::string PyStr::data() const { return raw; }

bool PyStr::operator==(const PyObj& other) const {
    if (const PyStr* s = dynamic_cast<const PyStr*>(&other))
        return raw == s->data();
    return false;
}
