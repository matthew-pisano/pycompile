//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builder_runtime.h"

#include <stdexcept>

Value* pyir_buildString(Value** parts, const int64_t count) {
    std::string result;
    for (int64_t i = 0; i < count; i++) {
        if (!parts[i]->isStr())
            throw std::runtime_error("BUILD_STRING: expected string part");
        result += std::get<std::string>(parts[i]->data);
    }
    return new Value(result);
}
