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
#include "pyruntime/objects/py_tuple.h"


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


PyListData valueToList(const PyObj* val) {
    if (const PyStr* pyString = dynamic_cast<const PyStr*>(val)) {
        PyListData result;
        for (const char c : pyString->data())
            result.push_back(new PyStr(c));
        return result;
    }
    if (const PySet* pySet = dynamic_cast<const PySet*>(val)) {
        PyListData result;
        for (PyObj* obj : pySet->data()) {
            obj->incref();
            result.push_back(obj);
        }
        return result;
    }
    if (const PyTuple* pyTuple = dynamic_cast<const PyTuple*>(val)) {
        PyListData result;
        for (PyObj* obj : pyTuple->data()) {
            obj->incref();
            result.push_back(obj);
        }
        return result;
    }
    if (const PyList* pyList = dynamic_cast<const PyList*>(val))
        return pyList->data();
    throw std::runtime_error(std::format("Cannot convert type '{}' to list", val->typeName()));
}


PySetData valueToSet(const PyObj* val) {
    if (const PyStr* pyString = dynamic_cast<const PyStr*>(val)) {
        PySetData result;
        for (const char c : pyString->data())
            result.insert(new PyStr(c));
        return result;
    }
    if (const PyList* pyList = dynamic_cast<const PyList*>(val)) {
        PySetData result;
        for (PyObj* obj : pyList->data()) {
            obj->incref();
            result.insert(obj);
        }
        return result;
    }
    if (const PyTuple* pyTuple = dynamic_cast<const PyTuple*>(val)) {
        PySetData result;
        for (PyObj* obj : pyTuple->data()) {
            obj->incref();
            result.insert(obj);
        }
        return result;
    }
    if (const PySet* pySet = dynamic_cast<const PySet*>(val))
        return pySet->data();
    throw std::runtime_error(std::format("Cannot convert type '{}' to set", val->typeName()));
}
