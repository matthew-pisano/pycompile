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


Value* pyir_makeFunction(void* fn_ptr) { return new Value(reinterpret_cast<Value::Function>(fn_ptr)); }


Value* pyir_call(const Value* callee, Value** args, const int64_t argc) {
    if (callee->isFn()) {
        ValueRef result(std::get<Value::Function>(callee->data)(args, argc));
        return result.release();
    }
    if (callee->isBoundMethod()) {
        const Value::BoundMethod& bm = std::get<Value::BoundMethod>(callee->data);
        ValueRef result(bm.fn(bm.self, args, argc));
        return result.release();
    }
    throw std::runtime_error("Object is not callable");
}


void pyir_decref(Value* v) {
    if (v)
        v->decref();
}


Value* pyir_pushNull() { return nullptr; }
