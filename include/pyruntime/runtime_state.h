//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_STATE_H
#define PYCOMPILE_RUNTIME_STATE_H
#include <unordered_map>
#include <vector>

#include "builtin_runtime.h"
#include "objects/py_function.h"

const std::unordered_map<std::string, PyFunctionData> builtinFuncs = {{"print", pyir_builtinPrint},
                                                                      {"len", pyir_builtinLen},
                                                                      {"int", pyir_builtinInt},
                                                                      {"float", pyir_builtinFloat},
                                                                      {"str", pyir_builtinStr},
                                                                      {"bool", pyir_builtinBool},
                                                                      {"list", pyir_builtinList},
                                                                      {"set", pyir_builtinSet},
                                                                      {"tuple", pyir_builtinTuple},
                                                                      {"dict", pyir_builtinDict},
                                                                      {"iter", pyir_builtinIter},
                                                                      {"next", pyir_builtinNext},
                                                                      {"enumerate", pyir_builtinEnumerate},
                                                                      {"isinstance", pyir_builtinIsInstance},
                                                                      {"range", pyir_builtinRange},
                                                                      {"type", pyir_builtinType},
                                                                      {"zip", pyir_builtinZip}};


using PyBuiltinVarFunction = PyObj* (*) ();
const std::unordered_map<std::string, PyBuiltinVarFunction> builtinVars = {{"__file__", pyir_builtinVarFile},
                                                                           {"__name__", pyir_builtinVarName}};

inline std::unordered_map<std::string, PyObj*> moduleScope;
inline std::vector<std::unordered_map<std::string, PyObj*>> scopeStack;


#endif // PYCOMPILE_RUNTIME_STATE_H
