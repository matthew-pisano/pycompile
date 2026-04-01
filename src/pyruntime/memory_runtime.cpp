//
// Created by matthew on 3/24/26.
//

#include "pyruntime/memory_runtime.h"

#include <format>
#include <stdexcept>
#include <unordered_map>

#include "pyruntime/function_runtime.h"
#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_method.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_state.h"


PyObj* pyir_loadFast(const char* name) {
    if (scopeStack.empty())
        throw std::runtime_error(std::string("No active scope to load '") + name + "'");

    const std::unordered_map<std::string, PyObj*>& locals = scopeStack.back();

    const auto it = locals.find(name);
    if (it == locals.end())
        throw std::runtime_error(std::string("Local variable '") + name + "' referenced before assignment");

    return it->second;
}


void pyir_storeFast(const char* name, PyObj* val) {
    if (scopeStack.empty())
        throw std::runtime_error(std::string("No active scope to store '") + name + "'");
    scopeStack.back()[name] = val;
}


PyObj* pyir_loadName(const char* name) {
    // Check for builtins
    if (const auto it = builtins.find(name); it != builtins.end())
        return new PyFunction(it->first, it->second);
    // Check for names in module scope
    if (const auto it = moduleScope.find(name); it != moduleScope.end()) {
        it->second->incref();
        return it->second;
    }
    throw std::runtime_error(std::string("name '") + name + "' is not defined");
}


void pyir_storeName(const char* name, PyObj* val) {
    if (const auto it = moduleScope.find(name); it != moduleScope.end())
        it->second->decref(); // Release old value
    val->incref();
    moduleScope[name] = val;
}


PyObj* pyir_loadConstStr(const char* str) { return new PyStr(std::string(str)); }


PyObj* pyir_loadConstInt(const int64_t val) { return new PyInt(val); }


PyObj* pyir_loadConstFloat(const double_t val) { return new PyFloat(val); }


PyObj* pyir_loadConstBool(const int8_t val) { return new PyBool(val == 1); }


PyObj* pyir_loadConstNone() { return new PyNone(); }


PyObj* pyir_loadConstTuple(PyObj** items, const int64_t count) {
    std::vector<PyObj*> result;
    result.reserve(count);
    for (int64_t i = 0; i < count; i++)
        result.push_back(items[i]);
    return new PyList(result);
}

PyObj* pyir_loadAttr(PyObj* obj, const char* name) { return obj->getAttr(name); }
