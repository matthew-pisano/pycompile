//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_FUNCTION_RUNTIME_H
#define PYCOMPILE_FUNCTION_RUNTIME_H

#include <string>

extern "C" {

struct PyObj;
struct PyFunction;

// scope management
void pyir_pushScope();

void pyir_popScope();

PyFunction* pyir_makeFunction(const char* fnName, void* fn_ptr);


// call dispatch
PyObj* pyir_call(PyObj* callee, PyObj** args, int64_t argc);

// Stub for Python push null
PyObj* pyir_pushNull();

// Decrease reference counting for v
void pyir_decref(PyObj* v);
}

#endif // PYCOMPILE_FUNCTION_RUNTIME_H
