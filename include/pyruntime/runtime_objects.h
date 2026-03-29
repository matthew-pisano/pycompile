//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_RUNTIME_OBJECTS_H
#define PYCOMPILE_RUNTIME_OBJECTS_H
#include <unordered_map>

#include "runtime_value.h"

struct PyIR_List {
    static const std::unordered_map<std::string, PyValue::BoundMethod::SelfFunction> attrs;

    static PyValue* append(PyValue* self, PyValue** args, int64_t argc);

    static PyValue* extend(PyValue* self, PyValue** args, int64_t argc);
};

#endif // PYCOMPILE_RUNTIME_OBJECTS_H
