//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_float.h"

std::string PyFloat::toString() const { return std::to_string(raw); }

std::string PyFloat::typeName() const { return "float"; }

double_t PyFloat::data() const { return raw; }

bool PyFloat::operator==(const PyFloat& other) const {
    return raw == other.raw;
}
