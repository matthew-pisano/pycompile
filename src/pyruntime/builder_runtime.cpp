//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builder_runtime.h"

#include <stdexcept>
#include <vector>


Value* pyir_buildString(Value** parts, const int64_t count) {
    std::string result;
    for (int64_t i = 0; i < count; i++) {
        if (!parts[i]->isStr())
            throw std::runtime_error("BUILD_STRING: expected string part");
        result += std::get<std::string>(parts[i]->data);
    }
    return new Value(result);
}


Value* pyir_buildList(Value** parts, const int64_t count) {
    Value::List result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.push_back(parts[i]);
    }
    return new Value(result);
}


void pyir_listExtend(Value* list, const Value* items) {
    if (!list->isList() || !items->isList())
        throw std::runtime_error("Can only extend list types with list types");

    Value::List& dest = std::get<Value::List>(list->data);
    const Value::List& src = std::get<Value::List>(items->data);
    for (Value* v : src)
        dest.push_back(v);
}


void pyir_listAppend(Value* list, Value* item) {
    if (!list->isList())
        throw std::runtime_error("Can only append to list types");

    Value::List& dest = std::get<Value::List>(list->data);
    dest.push_back(item);
}
