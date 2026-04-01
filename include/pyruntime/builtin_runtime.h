//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILTIN_RUNTIME_H
#define PYCOMPILE_BUILTIN_RUNTIME_H
#include <cstdint>

extern "C" {

struct PyObj;
struct PyNone;
struct PyInt;
struct PyBool;
struct PyStr;
struct PyFloat;
struct PyList;

PyNone* pyir_builtinPrint(PyObj** args, int64_t argc);

PyInt* pyir_builtinLen(PyObj** args, int64_t argc);

PyInt* pyir_builtinInt(PyObj** args, int64_t argc);

PyFloat* pyir_builtinFloat(PyObj** args, int64_t argc);

PyStr* pyir_builtinStr(PyObj** args, int64_t argc);

PyBool* pyir_builtinBool(PyObj** args, int64_t argc);

PyList* pyir_builtinList(PyObj** args, int64_t argc);
}

#endif // PYCOMPILE_BUILTIN_RUNTIME_H
