//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_STATE_H
#define PYCOMPILE_RUNTIME_STATE_H
#include <unordered_map>
#include <vector>

#include "builtin_runtime.h"
#include "pytypes/py_function.hpp"

/**
 * A mapping of built-in function names to their corresponding PyFunction objects.
 *
 * Instantiated by pyir_initModule and used by pyir_loadName to resolve built-in function references.
 */
inline std::unordered_map<std::string, PyFunction*> builtinFuncs;

using PyBuiltinVarFunction = PyObj* (*) ();

/**
 * A mapping of built-in variable names to their corresponding functions that return their values.
 */
const std::unordered_map<std::string, PyBuiltinVarFunction> builtinVars = {{"__file__", pyir_builtinVarFile},
                                                                           {"__name__", pyir_builtinVarName}};

/**
 * A stack of variable scopes, where each scope is a mapping of variable names to their corresponding PyObj* values.
 */
inline std::vector<std::unordered_map<std::string, PyObj*>> scopeStack;


#endif // PYCOMPILE_RUNTIME_STATE_H
