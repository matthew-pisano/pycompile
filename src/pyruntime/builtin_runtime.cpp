//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builtin_runtime.h"

#include <cmath>
#include <stdexcept>

#include "pyir/pyir_value.h"
#include "pyruntime/runtime_util.h"


Value* pyir_builtinPrint(Value** args, const int64_t argc) {
    for (int64_t i = 0; i < argc; i++) {
        if (i > 0)
            printf(" ");
        printf("%s", valueToString(args[i]).c_str());
    }
    printf("\n");
    return new Value(NoneType{});
}


Value* pyir_builtinLen(Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for len()");
    if (args[0]->isStr())
        return new Value(static_cast<int64_t>(std::get<std::string>(args[0]->data).size()));
    throw std::runtime_error("object has no len()");
}


Value* pyir_builtinInt(Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for int()");
    return std::visit([]<typename T>(const T& x) -> Value* {
        using ValType = std::decay_t<T>;
        if constexpr (std::is_same_v<ValType, int64_t>)
            return new Value(x);
        if constexpr (std::is_same_v<ValType, double_t>)
            return new Value(static_cast<int64_t>(x));
        if constexpr (std::is_same_v<ValType, bool>)
            return new Value(static_cast<int64_t>(x));
        if constexpr (std::is_same_v<ValType, std::string>)
            return new Value(static_cast<int64_t>(std::stoll(x)));
        throw std::runtime_error("Cannot convert to int()");
    }, args[0]->data);
}


Value* pyir_builtinFloat(Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for float()");
    return std::visit([]<typename T>(const T& x) -> Value* {
        using ValType = std::decay_t<T>;
        if constexpr (std::is_same_v<ValType, double_t>)
            return new Value(x);
        if constexpr (std::is_same_v<ValType, int64_t>)
            return new Value(static_cast<double_t>(x));
        if constexpr (std::is_same_v<ValType, bool>)
            return new Value(static_cast<double_t>(x));
        if constexpr (std::is_same_v<ValType, std::string>)
            return new Value(std::stod(x));
        throw std::runtime_error("cannot convert to float()");
    }, args[0]->data);
}


Value* pyir_builtinStr(Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for str()");
    return new Value(valueToString(args[0]));
}


Value* pyir_builtinBool(Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for bool()");
    return new Value(valueToBool(args[0]));
}
