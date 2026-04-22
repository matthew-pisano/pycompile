//
// Created by matthew on 3/24/26.
//

#include "pyruntime/memory_runtime.h"

#include <format>
#include <stdexcept>
#include <unordered_map>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_dict.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/objects/py_tuple.h"
#include "pyruntime/runtime_errors.h"
#include "pyruntime/runtime_state.h"


PyObj* pyir_loadFast(const char* name) {
    if (scopeStack.empty())
        throw std::runtime_error(std::string("No active scope to load '") + name + "'");

    const std::unordered_map<std::string, PyObj*>& locals = scopeStack.back();

    const auto it = locals.find(name);
    if (it == locals.end())
        throw PyNameError(std::string("Local variable '") + name + "' referenced before assignment");

    it->second->incref();
    return it->second;
}


void pyir_storeFast(const char* name, PyObj* val) {
    if (scopeStack.empty())
        throw std::runtime_error(std::string("No active scope to store '") + name + "'");
    scopeStack.back()[name] = val;
}


PyObj* pyir_loadName(const char* name) {
    // Check for builtin functions
    if (const auto it = builtinFuncs.find(name); it != builtinFuncs.end())
        return it->second;
    // Check for builtin variables
    if (const auto it = builtinVars.find(name); it != builtinVars.end())
        return it->second();
    // Check for names in module scope
    if (const auto it = scopeStack.back().find(name); it != scopeStack.back().end()) {
        it->second->incref();
        return it->second;
    }
    throw PyNameError(std::string("name '") + name + "' is not defined");
}


void pyir_storeName(const char* name, PyObj* val) {
    if (const auto it = scopeStack.back().find(name); it != scopeStack.back().end()) {
        if (it->second->decref()) // Release old value
            it->second = nullptr;
    }
    // Val already has at least one reference, no need to incref
    scopeStack.back()[name] = val;
}


PyObj* pyir_loadConstStr(const char* str) { return new PyStr(std::string(str)); }


PyObj* pyir_loadConstInt(const int64_t val) { return new PyInt(val); }


PyObj* pyir_loadConstFloat(const double_t val) { return new PyFloat(val); }


PyObj* pyir_loadConstBool(const int8_t val) { return val == 1 ? PyBool::True : PyBool::False; }


PyObj* pyir_loadConstNone() { return PyNone::None; }


PyObj* pyir_loadConstTuple(PyObj** items, const int64_t count) {
    std::vector<PyObj*> result;
    result.reserve(count);
    for (int64_t i = 0; i < count; i++)
        result.push_back(items[i]);
    return new PyTuple(result);
}


PyObj* pyir_loadAttr(PyObj* obj, const char* name) {
    try {
        PyObj* result = obj->getAttr(name);
        (void) obj->decref();
        return result;
    } catch (...) {
        (void) obj->decref();
        throw;
    }
}


void pyir_storeSubscr(PyObj* container, PyObj* idx, PyObj* value) {
    try {
        container->setIdx(idx, value);
        (void) idx->decref();
        (void) container->decref();
    } catch (...) {
        (void) idx->decref();
        (void) container->decref();
        (void) value->decref();
        throw;
    }
}
