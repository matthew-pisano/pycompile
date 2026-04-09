//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_MEMORY_RUNTIME_H
#define PYCOMPILE_MEMORY_RUNTIME_H
#include <cmath>

extern "C" {

struct PyObj;

PyObj* pyir_loadFast(const char* name);

void pyir_storeFast(const char* name, PyObj* val);

// name resolution, returns a builtin Fn or None
PyObj* pyir_loadName(const char* name);

// name storage in addition to builtins
void pyir_storeName(const char* name, PyObj* val);

PyObj* pyir_loadConstStr(const char* str);

PyObj* pyir_loadConstInt(int64_t val);

PyObj* pyir_loadConstFloat(double_t val);

PyObj* pyir_loadConstBool(int8_t val);

PyObj* pyir_loadConstNone();

PyObj* pyir_loadConstTuple(PyObj** items, int64_t count);

PyObj* pyir_loadAttr(PyObj* obj, const char* name);

void pyir_storeSubscr(PyObj* container, const PyObj* idx, PyObj* value);
}

#endif // PYCOMPILE_MEMORY_RUNTIME_H
