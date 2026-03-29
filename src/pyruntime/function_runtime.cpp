//
// Created by matthew on 3/24/26.
//

#include "pyruntime/function_runtime.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"
#include "pyruntime/runtime_state.h"

void pyir_pushScope() { scopeStack.emplace_back(); }


void pyir_popScope() {
    if (!scopeStack.empty())
        scopeStack.pop_back();
}


PyValue* pyir_makeFunction(void* fn_ptr) { return new PyValue(reinterpret_cast<PyValue::Function>(fn_ptr)); }


PyValue* pyir_call(const PyValue* callee, PyValue** args, const int64_t argc) {
    if (callee->isFn()) {
        ValueRef result(std::get<PyValue::Function>(callee->data)(args, argc));
        return result.release();
    }
    if (callee->isBoundMethod()) {
        const PyValue::BoundMethod& bm = std::get<PyValue::BoundMethod>(callee->data);
        ValueRef result(bm.fn(bm.self, args, argc));
        return result.release();
    }
    throw std::runtime_error("Object is not callable");
}


void pyir_decref(PyValue* v) {
    if (v)
        v->decref();
}


PyValue* pyir_pushNull() { return nullptr; }
