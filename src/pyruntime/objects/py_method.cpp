//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_method.h"

#include <format>

std::string PyMethod::toString() const { return std::format("<method {} of {} object>", fnName, self->typeName());  }

std::string PyMethod::typeName() const { return "function"; }

const std::unordered_map<std::string, PyMethod> PyMethod::attrs() const { return {}; }

PyFunctionType PyMethod::data() const { return fn; }
