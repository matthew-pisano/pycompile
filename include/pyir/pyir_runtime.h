//
// Created by matthew on 3/8/26.
//

#ifndef PYCOMPILE_PYIR_RUNTIME_H
#define PYCOMPILE_PYIR_RUNTIME_H

#include "pyir/pyir_value.h"

extern "C" {

// arithmetic
Value* pyir_add(const Value* lhs, const Value* rhs);

Value* pyir_sub(const Value* lhs, const Value* rhs);

Value* pyir_mul(const Value* lhs, const Value* rhs);

Value* pyir_div(const Value* lhs, const Value* rhs);

// comparison
Value* pyir_eq(const Value* lhs, const Value* rhs);

Value* pyir_ne(const Value* lhs, const Value* rhs);

Value* pyir_lt(const Value* lhs, const Value* rhs);

Value* pyir_le(const Value* lhs, const Value* rhs);

Value* pyir_gt(const Value* lhs, const Value* rhs);

Value* pyir_ge(const Value* lhs, const Value* rhs);

// name resolution, returns a builtin Fn or None
Value* pyir_load_name(const char* name);

// call dispatch
Value* pyir_call(const Value* callee, Value** args, int64_t argc);

// Stub for Python push null
Value* pyir_push_null();

// Decrease reference counting for v
void pyir_decref(Value* v);

Value* pyir_load_const_str(const char* str);

Value* pyir_load_const_int(const int64_t val);

// truthiness, used by conditional jumps
bool pyir_toBool(Value val);

// builtins
Value* pyir_builtinPrint(Value** args, int64_t argc);

Value* pyir_builtinLen(Value** args, int64_t argc);

Value* pyir_builtinInt(Value** args, int64_t argc);

Value* pyir_builtinFloat(Value** args, int64_t argc);

Value* pyir_builtinStr(Value** args, int64_t argc);

Value* pyir_builtinBool(Value** args, int64_t argc);

} //namespace pyir::runtime

#endif //PYCOMPILE_PYIR_RUNTIME_H
