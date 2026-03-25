//
// Created by matthew on 3/24/26.
//

#include "pyruntime/runtime_util.h"

#include <format>
#include <stdexcept>

double_t valueToFloat(const Value* v) {
    // Promote int to float if either operand is a float
    if (v->isFloat())
        return std::get<double_t>(v->data);
    if (v->isInt())
        return static_cast<double_t>(std::get<int64_t>(v->data));
    throw std::runtime_error("Cannot convert to float");
}

std::string valueToString(const Value* v) {
    return std::visit(
            []<typename T>(const T& x) -> std::string {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, NoneType>)
                    return "None";
                if constexpr (std::is_same_v<ValType, bool>)
                    return x ? "True" : "False";
                if constexpr (std::is_same_v<ValType, int64_t>)
                    return std::format("{}", x);
                if constexpr (std::is_same_v<ValType, double_t>)
                    return std::format("{}", x);
                if constexpr (std::is_same_v<ValType, std::string>)
                    return x;
                if constexpr (std::is_same_v<ValType, Value::Fn>)
                    return "<builtin>";
                return "<unknown>";
            },
            v->data);
}

bool valueToBool(const Value* val) {
    return std::visit(
            []<typename T>(const T& x) -> bool {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, NoneType>)
                    return false;
                if constexpr (std::is_same_v<ValType, bool>)
                    return x;
                if constexpr (std::is_same_v<ValType, int64_t>)
                    return x != 0;
                if constexpr (std::is_same_v<ValType, double_t>)
                    return x != 0.0;
                if constexpr (std::is_same_v<ValType, std::string>)
                    return !x.empty();
                if constexpr (std::is_same_v<ValType, Value::Fn>)
                    return true;
                return false;
            },
            val->data);
}
