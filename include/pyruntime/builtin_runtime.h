//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILTIN_RUNTIME_H
#define PYCOMPILE_BUILTIN_RUNTIME_H
#include <cstdint>

extern "C" {

struct PyObj;

PyObj* pyir_builtinVarName();

PyObj* pyir_builtinVarFile();

void pyir_initModule(const char* file, const char* name);

PyObj* pyir_builtinPrint(PyObj** args, int64_t argc);

PyObj* pyir_builtinLen(PyObj** args, int64_t argc);

PyObj* pyir_builtinInt(PyObj** args, int64_t argc);

PyObj* pyir_builtinFloat(PyObj** args, int64_t argc);

PyObj* pyir_builtinStr(PyObj** args, int64_t argc);

PyObj* pyir_builtinBool(PyObj** args, int64_t argc);

PyObj* pyir_builtinList(PyObj** args, int64_t argc);

PyObj* pyir_builtinSet(PyObj** args, int64_t argc);

PyObj* pyir_builtinTuple(PyObj** args, int64_t argc);

PyObj* pyir_builtinDict(PyObj** args, int64_t argc);

PyObj* pyir_builtinIter(PyObj** args, int64_t argc);

PyObj* pyir_builtinNext(PyObj** args, int64_t argc);

PyObj* pyir_builtinEnumerate(PyObj** args, int64_t argc);

PyObj* pyir_builtinIsInstance(PyObj** args, int64_t argc);

PyObj* pyir_builtinRange(PyObj** args, int64_t argc);

PyObj* pyir_builtinType(PyObj** args, int64_t argc);

PyObj* pyir_builtinZip(PyObj** args, int64_t argc);

PyObj* pyir_builtinInput(PyObj** args, int64_t argc);
}

#endif // PYCOMPILE_BUILTIN_RUNTIME_H
