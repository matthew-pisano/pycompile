//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_STATE_H
#define PYCOMPILE_RUNTIME_STATE_H
#include <unordered_map>
#include <vector>

#include "builtin_runtime.h"
#include "objects/py_function.h"

inline std::unordered_map<std::string, PyFunction*> builtinFuncs;

using PyBuiltinVarFunction = PyObj* (*) ();
const std::unordered_map<std::string, PyBuiltinVarFunction> builtinVars = {{"__file__", pyir_builtinVarFile},
                                                                           {"__name__", pyir_builtinVarName}};

inline std::vector<std::unordered_map<std::string, PyObj*>> scopeStack;


#endif // PYCOMPILE_RUNTIME_STATE_H
