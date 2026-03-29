//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_STATE_H
#define PYCOMPILE_RUNTIME_STATE_H
#include <unordered_map>
#include <vector>

#include "builtin_runtime.h"
#include "runtime_value.h"

const std::unordered_map<std::string, Value::Function> builtins = {
        {"print", pyir_builtinPrint}, {"len", pyir_builtinLen}, {"int", pyir_builtinInt},
        {"float", pyir_builtinFloat}, {"str", pyir_builtinStr}, {"bool", pyir_builtinBool},
        {"list", pyir_builtinList},
};

inline std::unordered_map<std::string, Value*> moduleScope;
inline std::vector<std::unordered_map<std::string, Value*>> scopeStack;


#endif // PYCOMPILE_RUNTIME_STATE_H
