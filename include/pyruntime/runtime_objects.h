//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_RUNTIME_OBJECTS_H
#define PYCOMPILE_RUNTIME_OBJECTS_H
#include <unordered_map>

#include "runtime_value.h"

struct PyIR_List {
    static const std::unordered_map<std::string, Value::BoundMethod::SelfFunction> attrs;

    static Value* append(Value* self, Value** args, int64_t argc);

    static Value* extend(Value* self, Value** args, int64_t argc);
};

#endif // PYCOMPILE_RUNTIME_OBJECTS_H
