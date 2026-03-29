//
// Created by matthew on 3/29/26.
//

#include "pyruntime/runtime_objects.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"


const std::unordered_map<std::string, PyBoundMethod::SelfFunction> PyIR_List::attrs = {
        {"append", append},
        {"extend", extend},
};


PyValue* PyIR_List::append(PyValue* self, PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("append() takes exactly one argument");
    pyir_listAppend(self, args[0]);
    return new PyValue(PyNone{});
}

PyValue* PyIR_List::extend(PyValue* self, PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("extend() takes exactly one argument");
    pyir_listExtend(self, args[0]);
    return new PyValue(PyNone{});
}
