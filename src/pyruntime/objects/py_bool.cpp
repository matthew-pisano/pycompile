//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_bool.h"

#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"

size_t PyBool::hash() const { return raw ? 1 : 0; }

std::string PyBool::toString() const { return raw ? "True" : "False"; }

std::string PyBool::typeName() const { return "bool"; }

bool PyBool::isTruthy() const { return raw; }

bool PyBool::data() const { return raw; }

std::partial_ordering PyBool::operator<=>(const PyObj& other) const noexcept {
    if (const PyBool* b = dynamic_cast<const PyBool*>(&other))
        return raw <=> b->data();
    if (const PyInt* i = dynamic_cast<const PyInt*>(&other))
        return static_cast<int64_t>(raw) <=> i->data();
    if (const PyFloat* f = dynamic_cast<const PyFloat*>(&other))
        return static_cast<double_t>(raw) <=> f->data();
    return std::partial_ordering::unordered;
}

bool PyBool::operator==(const PyObj& other) const noexcept {
    return (*this <=> other) == std::partial_ordering::equivalent;
}
