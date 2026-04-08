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

PyObj* PyStr::idx(const PyObj* idx) const {
    if (const PyInt* idxVal = dynamic_cast<const PyInt*>(idx)) {
        int64_t index = idxVal->data();
        if (index < 0)
            index += static_cast<int64_t>(raw.size());
        if (index < 0 || index >= static_cast<int64_t>(raw.size()))
            throw std::runtime_error("Str index out of range");
        return new PyStr(raw[index]);
    }
    throw std::runtime_error("Str indices must be integers");
}

std::string PyStr::toString() const { return raw; }

std::string PyStr::typeName() const { return "str"; }

bool PyStr::isTruthy() const { return !raw.empty(); }

std::string PyStr::data() const { return raw; }

std::partial_ordering PyStr::operator<=>(const PyObj& other) const noexcept {
    if (const PyStr* s = dynamic_cast<const PyStr*>(&other))
        return raw <=> s->data();
    return std::partial_ordering::unordered;
}

bool PyStr::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}
