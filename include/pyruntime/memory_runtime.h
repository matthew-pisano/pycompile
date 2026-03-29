//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_MEMORY_RUNTIME_H
#define PYCOMPILE_MEMORY_RUNTIME_H
#include <cmath>

#include "runtime_value.h"

extern "C" {

Value* pyir_loadFast(const char* name);

void pyir_storeFast(const char* name, Value* val);

// name resolution, returns a builtin Fn or None
Value* pyir_loadName(const char* name);

// name storage in addition to builtins
void pyir_storeName(const char* name, Value* val);

Value* pyir_loadConstStr(const char* str);

Value* pyir_loadConstInt(int64_t val);

Value* pyir_loadConstFloat(double_t val);

Value* pyir_loadConstBool(int8_t val);

Value* pyir_loadConstNone();

Value* pyir_loadConstTuple(Value** items, int64_t count);

Value* pyir_loadAttr(Value* obj, const char* name);
}

#endif // PYCOMPILE_MEMORY_RUNTIME_H
