//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_FUNCTION_RUNTIME_H
#define PYCOMPILE_FUNCTION_RUNTIME_H

#include <unordered_map>


#include "runtime_value.h"

extern "C" {

// scope management
void pyir_pushScope();

void pyir_popScope();

PyValue* pyir_makeFunction(void* fn_ptr);


// call dispatch
PyValue* pyir_call(const PyValue* callee, PyValue** args, int64_t argc);

// Stub for Python push null
PyValue* pyir_pushNull();

// Decrease reference counting for v
void pyir_decref(PyValue* v);
}

#endif // PYCOMPILE_FUNCTION_RUNTIME_H
