//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_none.h"

std::string PyNone::toString() const { return "None"; }

std::string PyNone::typeName() const { return "NoneType"; }

bool PyNone::isTruthy() const { return false; }
