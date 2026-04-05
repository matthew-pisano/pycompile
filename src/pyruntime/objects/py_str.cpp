//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_str.h"

#include "pyruntime/objects/py_int.h"

size_t PyStr::hash() const {
    constexpr std::hash<std::string> hasher;
    return hasher(raw);
}

PyInt* PyStr::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

std::string PyStr::toString() const { return raw; }

std::string PyStr::typeName() const { return "str"; }

bool PyStr::isTruthy() const { return !raw.empty(); }

std::string PyStr::data() const { return raw; }

bool PyStr::operator==(const PyObj& other) const {
    if (const PyStr* s = dynamic_cast<const PyStr*>(&other))
        return raw == s->data();
    return false;
}
