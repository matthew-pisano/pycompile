//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_float.h"

#include <format>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"

size_t PyFloat::hash() const { return std::bit_cast<size_t>(raw); }

std::string PyFloat::toString() const { return std::format("{}", raw); }

std::string PyFloat::typeName() const { return "float"; }

bool PyFloat::isTruthy() const { return raw != 0; }

double_t PyFloat::data() const { return raw; }

bool PyFloat::operator==(const PyObj& other) const {
    if (const PyBool* b = dynamic_cast<const PyBool*>(&other))
        return raw == b->data();
    if (const PyInt* i = dynamic_cast<const PyInt*>(&other))
        return raw == i->data();
    if (const PyFloat* f = dynamic_cast<const PyFloat*>(&other))
        return raw == f->data();
    return false;
}
