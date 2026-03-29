//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_bool.h"

std::string PyBool::toString() const { return std::to_string(raw); }

std::string PyBool::typeName() const { return "bool"; }

bool PyBool::data() const { return raw; }

bool PyBool::operator==(const PyBool& other) const {
    return raw == other.raw;
}
