//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_UTIL_H
#define PYCOMPILE_RUNTIME_UTIL_H
#include <cmath>

#include "pyir_value.h"

double_t valueToFloat(const Value* v);

std::string valueToString(const Value* v);

bool valueToBool(const Value* val);

#endif // PYCOMPILE_RUNTIME_UTIL_H
