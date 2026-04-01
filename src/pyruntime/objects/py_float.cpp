//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_float.h"

#include <format>

std::string PyFloat::toString() const { return std::format("{}", raw); }

std::string PyFloat::typeName() const { return "float"; }

bool PyFloat::isTruthy() const { return raw != 0; }

double_t PyFloat::data() const { return raw; }

bool PyFloat::operator==(const PyFloat& other) const { return raw == other.raw; }
