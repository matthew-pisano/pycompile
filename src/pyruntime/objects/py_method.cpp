//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_method.h"

#include <format>

std::string PyMethod::toString() const { return std::format("<method {} of {} object>", fnName, self->typeName()); }

std::string PyMethod::typeName() const { return "function"; }

bool PyMethod::isTruthy() const { return true; }

const std::unordered_map<std::string, PyMethod> PyMethod::attrs() const { return {}; }

PyObj* PyMethod::selfObj() const { return self; }

std::string PyMethod::funcName() const { return fnName; }

PyMethodType PyMethod::data() const { return fn; }
