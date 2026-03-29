//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_int.h"

std::string PyInt::toString() const { return std::to_string(raw); }

std::string PyInt::typeName() const { return "int"; }

int64_t PyInt::data() const { return raw; }

bool PyInt::operator==(const PyInt& other) const {
    return raw == other.raw;
}
