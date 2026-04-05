//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_STATE_H
#define PYCOMPILE_RUNTIME_STATE_H
#include <unordered_map>
#include <vector>

#include "builtin_runtime.h"
#include "objects/py_function.h"

const std::unordered_map<std::string, PyFunctionData> builtins = {
        {"print", pyir_builtinPrint}, {"len", pyir_builtinLen}, {"int", pyir_builtinInt},
        {"float", pyir_builtinFloat}, {"str", pyir_builtinStr}, {"bool", pyir_builtinBool},
        {"list", pyir_builtinList},   {"set", pyir_builtinSet},
};

inline std::unordered_map<std::string, PyObj*> moduleScope;
inline std::vector<std::unordered_map<std::string, PyObj*>> scopeStack;


#endif // PYCOMPILE_RUNTIME_STATE_H
