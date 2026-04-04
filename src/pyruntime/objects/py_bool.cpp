//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_bool.h"

#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"

std::string PyBool::toString() const { return raw ? "True" : "False"; }

std::string PyBool::typeName() const { return "bool"; }

bool PyBool::isTruthy() const { return raw; }

bool PyBool::data() const { return raw; }

bool PyBool::operator==(const PyObj& other) const {
    if (const PyBool* b = dynamic_cast<const PyBool*>(&other))
        return raw == b->data();
    if (const PyInt* i = dynamic_cast<const PyInt*>(&other))
        return raw == i->data();
    if (const PyFloat* f = dynamic_cast<const PyFloat*>(&other))
        return raw == f->data();
    return false;
}
