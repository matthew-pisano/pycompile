//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_function.h"

#include <format>

size_t PyFunction::hash() const { throw std::runtime_error("Unhashable type " + typeName()); }

std::string PyFunction::toString() const { return std::format("<function {}>", fnName); }

std::string PyFunction::typeName() const { return "function"; }

bool PyFunction::isTruthy() const { return true; }

std::string PyFunction::funcName() const { return fnName; }

PyFunctionData PyFunction::data() const { return fn; }

std::partial_ordering PyFunction::operator<=>(const PyObj&) const noexcept { return std::partial_ordering::unordered; }
