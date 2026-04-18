//
// Created by matthew on 3/24/26.
//

#include "pyruntime/function_runtime.h"

#include <format>
#include <stdexcept>

#include "pyruntime/runtime_errors.h"
#include "pyruntime/runtime_state.h"

void pyir_pushScope() { scopeStack.emplace_back(); }


void pyir_popScope() {
    if (!scopeStack.empty())
        scopeStack.pop_back();
}


PyFunction* pyir_makeFunction(const char* fnName, void* fn_ptr) {
    return new PyFunction(fnName, reinterpret_cast<PyFunctionData>(fn_ptr));
}


PyObj* pyir_call(const PyObj* callee, PyObj** args, const int64_t argc) {
    if (const PyFunction* func = dynamic_cast<const PyFunction*>(callee)) {
        PyObjRef result(func->data()(args, argc));
        return result.release();
    }
    if (const PyMethod* method = dynamic_cast<const PyMethod*>(callee)) {
        PyObjRef result(method->data()(method->selfObj(), args, argc));
        return result.release();
    }
    throw PyTypeError(std::format("'{}' object is not callable", callee->typeName()));
}


void pyir_decref(PyObj* v) {
    if (v)
        v->decref();
}


PyObj* pyir_pushNull() { return nullptr; }
