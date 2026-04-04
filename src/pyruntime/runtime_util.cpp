//
// Created by matthew on 3/24/26.
//

#include "pyruntime/runtime_util.h"

#include <algorithm>
#include <format>
#include <stdexcept>

#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_str.h"


bool vectorContains(const std::vector<PyObj*>& vec, const PyObj* item) {
    return std::ranges::any_of(vec, [item](const PyObj* elem) { return *elem == *item; });
}


bool unorderedSetContains(const std::unordered_set<PyObj*>& set, const PyObj* item) {
    return std::ranges::any_of(set, [item](const PyObj* elem) { return *elem == *item; });
}


bool vectorEquality(const std::vector<PyObj*>& vec1, const std::vector<PyObj*>& vec2) {
    if (vec1.size() != vec2.size())
        return false;
    return std::ranges::all_of(vec1, [vec2](const PyObj* elem) { return vectorContains(vec2, elem); });
}


bool unorderedSetEquality(const std::unordered_set<PyObj*>& set1, const std::unordered_set<PyObj*>& set2) {
    if (set1.size() != set2.size())
        return false;
    return std::ranges::all_of(set1, [set2](const PyObj* elem) { return unorderedSetContains(set2, elem); });
}


double_t valueToFloat(const PyObj* val) {
    if (const PyFloat* pyFloat = dynamic_cast<const PyFloat*>(val))
        return pyFloat->data();
    if (const PyInt* pyInt = dynamic_cast<const PyInt*>(val))
        return static_cast<double_t>(pyInt->data());
    throw std::runtime_error(std::format("Cannot convert type '{}' to float", val->typeName()));
}


std::string valueToString(const PyObj* val, const bool quoteStrings) {
    if (const PyStr* pyString = dynamic_cast<const PyStr*>(val))
        return quoteStrings ? "'" + pyString->data() + "'" : pyString->data();
    return val->toString();
}


std::vector<PyObj*> valueToList(const PyObj* val) {
    if (const PyStr* pyString = dynamic_cast<const PyStr*>(val)) {
        std::vector<PyObj*> result;
        for (const char c : pyString->data())
            result.push_back(new PyStr(c));
        return result;
    }
    if (const PySet* pySet = dynamic_cast<const PySet*>(val)) {
        std::vector<PyObj*> result;
        for (PyObj* obj : pySet->data()) {
            obj->incref();
            result.push_back(obj);
        }
        return result;
    }
    if (const PyList* pyList = dynamic_cast<const PyList*>(val))
        return pyList->data();
    throw std::runtime_error(std::format("Cannot convert type '{}' to list", val->typeName()));
}


std::unordered_set<PyObj*> valueToSet(const PyObj* val) {
    if (const PyStr* pyString = dynamic_cast<const PyStr*>(val)) {
        std::unordered_set<PyObj*> result;
        for (const char c : pyString->data())
            result.insert(new PyStr(c));
        return result;
    }
    if (const PyList* pyList = dynamic_cast<const PyList*>(val)) {
        std::unordered_set<PyObj*> result;
        for (PyObj* obj : pyList->data()) {
            obj->incref();
            result.insert(obj);
        }
        return result;
    }
    if (const PySet* pySet = dynamic_cast<const PySet*>(val))
        return pySet->data();
    throw std::runtime_error(std::format("Cannot convert type '{}' to set", val->typeName()));
}
