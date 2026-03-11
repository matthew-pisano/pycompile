//
// Created by matthew on 3/8/26.
//

#ifndef PYCOMPILE_PYIR_RUNTIME_H
#define PYCOMPILE_PYIR_RUNTIME_H

#include "pyir/pyir_value.h"

namespace pyir::runtime {

    // arithmetic
    Value add(const Value& lhs, const Value& rhs);

    Value sub(const Value& lhs, const Value& rhs);

    Value mul(const Value& lhs, const Value& rhs);

    Value div(const Value& lhs, const Value& rhs);

    // comparison
    Value eq(const Value& lhs, const Value& rhs);

    Value ne(const Value& lhs, const Value& rhs);

    Value lt(const Value& lhs, const Value& rhs);

    Value le(const Value& lhs, const Value& rhs);

    Value gt(const Value& lhs, const Value& rhs);

    Value ge(const Value& lhs, const Value& rhs);

    // name resolution, returns a builtin Fn or None
    Value loadName(const char* name);

    // call dispatch
    Value call(const Value& callee, Value* args, int64_t argc);

    // truthiness, used by conditional jumps
    bool toBool(Value val);

    // builtins
    Value builtinPrint(const Value* args, int64_t argc);

    Value builtinLen(Value* args, int64_t argc);

    Value builtinInt(Value* args, int64_t argc);

    Value builtinFloat(Value* args, int64_t argc);

    Value builtinStr(const Value* args, int64_t argc);

    Value builtinBool(Value* args, int64_t argc);

} //namespace pyir::runtime

#endif //PYCOMPILE_PYIR_RUNTIME_H
