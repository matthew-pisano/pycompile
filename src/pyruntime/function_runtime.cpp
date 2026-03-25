//
// Created by matthew on 3/24/26.
//

#include "pyruntime/function_runtime.h"

#include <stdexcept>

#include "pyruntime/runtime_state.h"


void pyir_pushScope() {
    scopeStack.emplace_back();
}


void pyir_popScope() {
    if (!scopeStack.empty())
        scopeStack.pop_back();
}


Value* pyir_makeFunction(void* fn_ptr) {
    return new Value(reinterpret_cast<Value::Fn>(fn_ptr));
}


Value* pyir_call(const Value* callee, Value** args, const int64_t argc) {
    if (!callee->isFn())
        throw std::runtime_error("object is not callable");
    // If the fn throws, result never gets returned, leak if anything was allocated
    ValueRef result(std::get<Value::Fn>(callee->data)(args, argc));
    return result.release();
}


void pyir_decref(Value* v) {
    if (v)
        v->decref();
}


Value* pyir_pushNull() {
    return nullptr;
}

