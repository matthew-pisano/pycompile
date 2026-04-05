//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_UTIL_H
#define PYCOMPILE_RUNTIME_UTIL_H
#include <cmath>
#include <string>
#include <unordered_set>
#include <vector>

#include "objects/py_list.h"
#include "objects/py_object.h"
#include "objects/py_set.h"

bool vectorContains(const PyListData& vec, const PyObj* item);

bool unorderedSetContains(const PySetData& set, const PyObj* item);

bool vectorEquality(const PyListData& vec1, const PyListData& vec2);

bool unorderedSetEquality(const PySetData& set1, const PySetData& set2);

double_t valueToFloat(const PyObj* val);

std::string valueToString(const PyObj* val, bool quoteStrings = false);

PyListData valueToList(const PyObj* val);

PySetData valueToSet(const PyObj* val);

#endif // PYCOMPILE_RUNTIME_UTIL_H
