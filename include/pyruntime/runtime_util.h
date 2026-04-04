//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_UTIL_H
#define PYCOMPILE_RUNTIME_UTIL_H
#include <cmath>
#include <string>
#include <unordered_set>
#include <vector>


struct PyObj;

double_t valueToFloat(const PyObj* val);

std::string valueToString(const PyObj* val, bool quoteStrings = false);

std::vector<PyObj*> valueToList(const PyObj* val);

std::unordered_set<PyObj*> valueToSet(const PyObj* val);

#endif // PYCOMPILE_RUNTIME_UTIL_H
