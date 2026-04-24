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
#include "pyruntime/runtime_errors.h"


void decrefArgs(PyObj** args, const int64_t argc) {
    for (int64_t i = 0; i < argc; i++) {
        if (args[i]->decref())
            args[i] = nullptr;
    }
}


std::string formatBadConversion(const std::string& valType, const std::string& type, const std::string& valRepr) {
    return std::format("Could not convert {} to {}: '{}'", valType, type, valRepr);
}


double_t valueToFloat(const PyObj* val) {
    if (const PyFloat* pyFloat = dynamic_cast<const PyFloat*>(val))
        return pyFloat->data();
    if (const PyInt* pyInt = dynamic_cast<const PyInt*>(val))
        return static_cast<double_t>(pyInt->data());
    throw PyTypeError(formatBadConversion(val->typeName(), "float", val->toString()));
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
    if (const PyList* pyList = dynamic_cast<const PyList*>(val)) {
        PyListData result;
        for (PyObj* obj : pyList->data()) {
            obj->incref();
            result.push_back(obj);
        }
        return result;
    }
    throw PyTypeError(formatBadConversion(val->typeName(), "list", val->toString()));
}


PySetData valueToSet(const PyObj* val) {
    if (const PyStr* pyString = dynamic_cast<const PyStr*>(val)) {
        PySetData result;
        for (const char c : pyString->data())
            if (PyStr* charStr = new PyStr(c); !result.insert(charStr).second)
                (void) charStr->decref(); // Decref string if it is not added to the set
        return result;
    }
    if (const PyList* pyList = dynamic_cast<const PyList*>(val)) {
        PySetData result;
        for (PyObj* obj : pyList->data()) {
            if (result.insert(obj).second)
                obj->incref();
        }
        return result;
    }
    if (const PyTuple* pyTuple = dynamic_cast<const PyTuple*>(val)) {
        PySetData result;
        for (PyObj* obj : pyTuple->data()) {
            if (result.insert(obj).second)
                obj->incref();
        }
        return result;
    }
    if (const PySet* pySet = dynamic_cast<const PySet*>(val)) {
        PySetData result;
        for (PyObj* obj : pySet->data()) {
            if (result.insert(obj).second)
                obj->incref();
        }
        return result;
    }
    throw PyTypeError(formatBadConversion(val->typeName(), "set", val->toString()));
}
