//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builtin_runtime.h"

#include <cmath>
#include <stdexcept>

#include "pyruntime/runtime_util.h"
#include "pyruntime/runtime_value.h"


PyValue* pyir_builtinPrint(PyValue** args, const int64_t argc) {
    for (int64_t i = 0; i < argc; i++) {
        if (i > 0)
            printf(" ");
        printf("%s", valueToString(args[i]).c_str());
    }
    printf("\n");
    return new PyValue(PyNone{});
}


PyValue* pyir_builtinLen(PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for len()");
    if (args[0]->isStr())
        return new PyValue(static_cast<int64_t>(std::get<std::string>(args[0]->data).size()));
    if (args[0]->isList())
        return PyList::len(args[0]);
    throw std::runtime_error("Object has no len()");
}


PyValue* pyir_builtinInt(PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for int()");
    return std::visit(
            []<typename T>(const T& x) -> PyValue* {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, int64_t>)
                    return new PyValue(x);
                if constexpr (std::is_same_v<ValType, double_t>)
                    return new PyValue(static_cast<int64_t>(x));
                if constexpr (std::is_same_v<ValType, bool>)
                    return new PyValue(static_cast<int64_t>(x));
                if constexpr (std::is_same_v<ValType, std::string>)
                    return new PyValue(static_cast<int64_t>(std::stoll(x)));
                throw std::runtime_error("Cannot convert to int()");
            },
            args[0]->data);
}


PyValue* pyir_builtinFloat(PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for float()");
    return std::visit(
            []<typename T>(const T& x) -> PyValue* {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, double_t>)
                    return new PyValue(x);
                if constexpr (std::is_same_v<ValType, int64_t>)
                    return new PyValue(static_cast<double_t>(x));
                if constexpr (std::is_same_v<ValType, bool>)
                    return new PyValue(static_cast<double_t>(x));
                if constexpr (std::is_same_v<ValType, std::string>)
                    return new PyValue(std::stod(x));
                throw std::runtime_error("cannot convert to float()");
            },
            args[0]->data);
}


PyValue* pyir_builtinStr(PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for str()");
    return new PyValue(valueToString(args[0]));
}


PyValue* pyir_builtinBool(PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for bool()");
    return new PyValue(valueToBool(args[0]));
}


PyValue* pyir_builtinList(PyValue** args, const int64_t argc) {
    if (argc > 1)
        throw std::runtime_error("Too many arguments for list()");
    if (argc == 0)
        return new PyValue(PyList{});

    return new PyValue(valueToList(args[0]));
}
