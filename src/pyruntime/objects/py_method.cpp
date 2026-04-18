//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_method.h"

#include <format>

#include "pyruntime/runtime_errors.h"

size_t PyMethod::hash() const { throw PyTypeError("Unhashable type " + typeName()); }

std::string PyMethod::toString() const { return std::format("<method {} of {} object>", fnName, self->typeName()); }

std::string PyMethod::typeName() const { return "function"; }

bool PyMethod::isTruthy() const { return true; }

PyObj* PyMethod::selfObj() const { return self; }

std::string PyMethod::funcName() const { return fnName; }

PyMethodData PyMethod::data() const { return fn; }

std::partial_ordering PyMethod::operator<=>(const PyObj&) const noexcept { return std::partial_ordering::unordered; }

bool PyMethod::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}
