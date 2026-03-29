//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_UTIL_H
#define PYCOMPILE_RUNTIME_UTIL_H
#include <cmath>

#include "runtime_value.h"

double_t valueToFloat(const PyValue* val);

std::string valueToString(const PyValue* val, bool quoteStrings = false);

bool valueToBool(const PyValue* val);

PyValue::List valueToList(const PyValue* val);

#endif // PYCOMPILE_RUNTIME_UTIL_H
