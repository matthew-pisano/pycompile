//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_none.h"

size_t PyNone::hash() const { return 0; }

std::string PyNone::toString() const { return "None"; }

std::string PyNone::typeName() const { return "NoneType"; }

bool PyNone::isTruthy() const { return false; }

std::partial_ordering PyNone::operator<=>(const PyObj& other) const noexcept {
    if (dynamic_cast<const PyNone*>(&other))
        return std::partial_ordering::equivalent;
    return std::partial_ordering::unordered;
}

bool PyNone::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}
