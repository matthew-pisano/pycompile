//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_int.h"

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_float.h"

size_t PyInt::hash() const { return static_cast<size_t>(raw); }

std::string PyInt::toString() const { return std::to_string(raw); }

std::string PyInt::typeName() const { return "int"; }

bool PyInt::isTruthy() const { return raw != 0; }

int64_t PyInt::data() const { return raw; }

std::partial_ordering PyInt::operator<=>(const PyObj& other) const noexcept {
    if (const PyBool* b = dynamic_cast<const PyBool*>(&other))
        return raw <=> static_cast<int64_t>(b->data());
    if (const PyInt* i = dynamic_cast<const PyInt*>(&other))
        return raw <=> i->data();
    if (const PyFloat* f = dynamic_cast<const PyFloat*>(&other))
        return static_cast<double_t>(raw) <=> f->data();
    return std::partial_ordering::unordered;
}
