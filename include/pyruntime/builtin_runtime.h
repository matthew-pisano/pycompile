//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILTIN_RUNTIME_H
#define PYCOMPILE_BUILTIN_RUNTIME_H

#include "runtime_value.h"

extern "C" {

PyValue* pyir_builtinPrint(PyValue** args, int64_t argc);

PyValue* pyir_builtinLen(PyValue** args, int64_t argc);

PyValue* pyir_builtinInt(PyValue** args, int64_t argc);

PyValue* pyir_builtinFloat(PyValue** args, int64_t argc);

PyValue* pyir_builtinStr(PyValue** args, int64_t argc);

PyValue* pyir_builtinBool(PyValue** args, int64_t argc);

PyValue* pyir_builtinList(PyValue** args, int64_t argc);
}

#endif // PYCOMPILE_BUILTIN_RUNTIME_H
