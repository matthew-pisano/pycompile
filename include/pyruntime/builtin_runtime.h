//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILTIN_RUNTIME_H
#define PYCOMPILE_BUILTIN_RUNTIME_H
#include "pyir_value.h"

extern "C" {


Value* pyir_builtinPrint(Value** args, int64_t argc);

Value* pyir_builtinLen(Value** args, int64_t argc);

Value* pyir_builtinInt(Value** args, int64_t argc);

Value* pyir_builtinFloat(Value** args, int64_t argc);

Value* pyir_builtinStr(Value** args, int64_t argc);

Value* pyir_builtinBool(Value** args, int64_t argc);
}

#endif // PYCOMPILE_BUILTIN_RUNTIME_H
