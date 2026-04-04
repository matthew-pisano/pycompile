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

bool vectorContains(const std::vector<PyObj*>& vec, const PyObj* item);

bool unorderedSetContains(const std::unordered_set<PyObj*>& set, const PyObj* item);

bool vectorEquality(const std::vector<PyObj*>& vec1, const std::vector<PyObj*>& vec2);

bool unorderedSetEquality(const std::unordered_set<PyObj*>& set1, const std::unordered_set<PyObj*>& set2);

double_t valueToFloat(const PyObj* val);

std::string valueToString(const PyObj* val, bool quoteStrings = false);

std::vector<PyObj*> valueToList(const PyObj* val);

std::unordered_set<PyObj*> valueToSet(const PyObj* val);

#endif // PYCOMPILE_RUNTIME_UTIL_H
