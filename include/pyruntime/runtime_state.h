//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_STATE_H
#define PYCOMPILE_RUNTIME_STATE_H
#include <unordered_map>
#include <vector>

#include "builtin_runtime.h"
#include "objects/py_function.h"

inline std::unordered_map<std::string, PyFunction*> builtinFuncs = {
        {"print", new PyFunction("print", pyir_builtinPrint)},
        {"len", new PyFunction("len", pyir_builtinLen)},
        {"int", new PyFunction("int", pyir_builtinInt)},
        {"float", new PyFunction("float", pyir_builtinFloat)},
        {"str", new PyFunction("str", pyir_builtinStr)},
        {"bool", new PyFunction("bool", pyir_builtinBool)},
        {"list", new PyFunction("list", pyir_builtinList)},
        {"set", new PyFunction("set", pyir_builtinSet)},
        {"tuple", new PyFunction("tuple", pyir_builtinTuple)},
        {"dict", new PyFunction("dict", pyir_builtinDict)},
        {"iter", new PyFunction("iter", pyir_builtinIter)},
        {"next", new PyFunction("next", pyir_builtinNext)},
        {"enumerate", new PyFunction("enumerate", pyir_builtinEnumerate)},
        {"isinstance", new PyFunction("isinstance", pyir_builtinIsInstance)},
        {"range", new PyFunction("range", pyir_builtinRange)},
        {"type", new PyFunction("type", pyir_builtinType)},
        {"zip", new PyFunction("zip", pyir_builtinZip)},
        {"input", new PyFunction("input", pyir_builtinInput)}};


using PyBuiltinVarFunction = PyObj* (*) ();
const std::unordered_map<std::string, PyBuiltinVarFunction> builtinVars = {{"__file__", pyir_builtinVarFile},
                                                                           {"__name__", pyir_builtinVarName}};

inline std::unordered_map<std::string, PyObj*> moduleScope;
inline std::vector<std::unordered_map<std::string, PyObj*>> scopeStack;


#endif // PYCOMPILE_RUNTIME_STATE_H
