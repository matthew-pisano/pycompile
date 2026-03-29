//
// Created by matthew on 3/24/26.
//

#include "pyruntime/runtime_util.h"

#include <format>
#include <stdexcept>


double_t valueToFloat(const PyValue* val) {
    if (val->isFloat())
        return std::get<double_t>(val->data);
    if (val->isInt())
        return static_cast<double_t>(std::get<int64_t>(val->data));
    throw std::runtime_error("Cannot convert to float");
}


std::string valueToString(const PyValue* val, bool quoteStrings) {
    return std::visit(
            [quoteStrings]<typename T>(const T& x) -> std::string {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, PyValue::NoneType>)
                    return "None";
                if constexpr (std::is_same_v<ValType, bool>)
                    return x ? "True" : "False";
                if constexpr (std::is_same_v<ValType, int64_t>)
                    return std::format("{}", x);
                if constexpr (std::is_same_v<ValType, double_t>)
                    return std::format("{}", x);
                if constexpr (std::is_same_v<ValType, std::string>)
                    return quoteStrings ? ("'" + x + "'") : x;
                if constexpr (std::is_same_v<ValType, PyValue::Function>)
                    return "<callable>";
                if constexpr (std::is_same_v<ValType, PyValue::BoundMethod>)
                    return "<built-in method>";
                if constexpr (std::is_same_v<ValType, PyValue::List>) {
                    if (x.empty())
                        return "[]";

                    std::string result = "[";
                    for (size_t i = 0; i < x.size() - 1; i++)
                        result += valueToString(x[i], true) + ", ";
                    result += valueToString(x[x.size() - 1], true) + "]";
                    return result;
                }
                throw std::runtime_error("Unable to convert type to string");
            },
            val->data);
}


bool valueToBool(const PyValue* val) {
    return std::visit(
            []<typename T>(const T& x) -> bool {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, PyValue::NoneType>)
                    return false;
                if constexpr (std::is_same_v<ValType, bool>)
                    return x;
                if constexpr (std::is_same_v<ValType, int64_t>)
                    return x != 0;
                if constexpr (std::is_same_v<ValType, double_t>)
                    return x != 0.0;
                if constexpr (std::is_same_v<ValType, std::string>)
                    return !x.empty();
                if constexpr (std::is_same_v<ValType, PyValue::Function>)
                    return true;
                if constexpr (std::is_same_v<ValType, PyValue::BoundMethod>)
                    return true;
                if constexpr (std::is_same_v<ValType, PyValue::List>)
                    return x.size() > 0;
                return false;
            },
            val->data);
}


PyValue::List valueToList(const PyValue* val) {
    return std::visit(
            []<typename T>(const T& x) -> PyValue::List {
                using ValType = std::decay_t<T>;
                if constexpr (std::is_same_v<ValType, std::string>) {
                    PyValue::List result;
                    for (const char c : x)
                        result.push_back(new PyValue(c));
                    return result;
                }
                if constexpr (std::is_same_v<ValType, PyValue::List>) {
                    return x;
                }
                throw std::runtime_error("Unable to convert type to list");
            },
            val->data);
}
