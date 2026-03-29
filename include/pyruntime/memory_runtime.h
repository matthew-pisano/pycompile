//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_MEMORY_RUNTIME_H
#define PYCOMPILE_MEMORY_RUNTIME_H
#include <cmath>

#include "runtime_value.h"

extern "C" {

PyValue* pyir_loadFast(const char* name);

void pyir_storeFast(const char* name, PyValue* val);

// name resolution, returns a builtin Fn or None
PyValue* pyir_loadName(const char* name);

// name storage in addition to builtins
void pyir_storeName(const char* name, PyValue* val);

PyValue* pyir_loadConstStr(const char* str);

PyValue* pyir_loadConstInt(int64_t val);

PyValue* pyir_loadConstFloat(double_t val);

PyValue* pyir_loadConstBool(int8_t val);

PyValue* pyir_loadConstNone();

PyValue* pyir_loadConstTuple(PyValue** items, int64_t count);

PyValue* pyir_loadAttr(PyValue* obj, const char* name);
}

#endif // PYCOMPILE_MEMORY_RUNTIME_H
