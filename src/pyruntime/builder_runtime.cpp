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
    std::vector<Value*> result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.push_back(parts[i]);
    }
    return new Value(result);
}
