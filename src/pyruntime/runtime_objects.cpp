//
// Created by matthew on 3/29/26.
//

#include "pyruntime/runtime_objects.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"


const std::unordered_map<std::string, Value::BoundMethod::SelfFunction> PyIR_List::attrs = {
        {"append", append},
        {"extend", extend},
};


Value* PyIR_List::append(Value* self, Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("append() takes exactly one argument");
    pyir_listAppend(self, args[0]);
    return new Value(Value::NoneType{});
}

Value* PyIR_List::extend(Value* self, Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("extend() takes exactly one argument");
    pyir_listExtend(self, args[0]);
    return new Value(Value::NoneType{});
}
