//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_int.h"

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_float.h"

std::string PyInt::toString() const { return std::to_string(raw); }

std::string PyInt::typeName() const { return "int"; }

bool PyInt::isTruthy() const { return raw != 0; }

int64_t PyInt::data() const { return raw; }

bool PyInt::operator==(const PyObj& other) const {
    if (const PyBool* b = dynamic_cast<const PyBool*>(&other))
        return raw == b->data();
    if (const PyInt* i = dynamic_cast<const PyInt*>(&other))
        return raw == i->data();
    if (const PyFloat* f = dynamic_cast<const PyFloat*>(&other))
        return raw == f->data();
    return false;
}
