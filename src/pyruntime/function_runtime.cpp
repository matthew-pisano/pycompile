//
// Created by matthew on 3/24/26.
//

#include "pyruntime/function_runtime.h"

#include <format>
#include <ranges>
#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/runtime_errors.h"
#include "pyruntime/runtime_state.h"

void pyir_pushScope() { scopeStack.emplace_back(); }


void pyir_popScope() {
    if (scopeStack.empty())
        return;

    std::unordered_map<std::string, PyObj*> scope = scopeStack.back();
    for (PyObj*& obj : scope | std::views::values) {
        if (!obj)
            continue; // Skip already nulled objects
        if (dynamic_cast<PyNone*>(obj) || dynamic_cast<PyBool*>(obj))
            continue; // Skip immortal objects
        if (obj->decref())
            obj = nullptr;
    }
    scopeStack.pop_back();
}


PyFunction* pyir_makeFunction(const char* fnName, void* fn_ptr) {
    return new PyFunction(fnName, reinterpret_cast<PyFunctionData>(fn_ptr));
}


PyObj* pyir_call(PyObj* callee, PyObj** args, const int64_t argc) {
    if (const PyFunction* func = dynamic_cast<const PyFunction*>(callee)) {
        // No need to incref for arguments to builtins
        if (!builtinFuncs.contains(func->funcName())) {
            // Incref arguments before being passed
            for (int64_t i = 0; i < argc; i++)
                args[i]->incref();
            (void) callee->decref();
        }

        PyObjRef result(func->data()(args, argc));
        return result.release();
    }
    if (const PyMethod* method = dynamic_cast<const PyMethod*>(callee)) {
        PyObjRef result(method->data()(method->selfObj(), args, argc));
        for (int64_t i = 0; i < argc; i++)
            if (args[i]->decref())
                args[i] = nullptr;
        (void) callee->decref();
        return result.release();
    }
    throw PyTypeError(std::format("'{}' object is not callable", callee->typeName()));
}


void pyir_decref(PyObj* v) {
    if (v)
        (void) v->decref();
}


PyObj* pyir_pushNull() { return nullptr; }
