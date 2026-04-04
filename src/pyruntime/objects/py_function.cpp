//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_function.h"

#include <format>

std::string PyFunction::toString() const { return std::format("<function {}>", fnName); }

std::string PyFunction::typeName() const { return "function"; }

bool PyFunction::isTruthy() const { return true; }

std::string PyFunction::funcName() const { return fnName; }

PyFunctionType PyFunction::data() const { return fn; }

bool PyFunction::operator==(const PyObj&) const { return false; }
