//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_FUNCTION_RUNTIME_H
#define PYCOMPILE_FUNCTION_RUNTIME_H
#include "pyir_value.h"

extern "C" {

// scope management
void pyir_pushScope();

void pyir_popScope();

Value* pyir_makeFunction(void* fn_ptr);


// call dispatch
Value* pyir_call(const Value* callee, Value** args, int64_t argc);

// Stub for Python push null
Value* pyir_pushNull();

// Decrease reference counting for v
void pyir_decref(Value* v);
}

#endif // PYCOMPILE_FUNCTION_RUNTIME_H
