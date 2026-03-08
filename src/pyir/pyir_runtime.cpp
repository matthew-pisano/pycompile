//
// Created by matthew on 3/8/26.
//

#include "pyir/pyir_runtime.h"

#include <csignal>
#include <stdexcept>
#include <unordered_map>


namespace pyir::runtime {

    static double toFloat(const Value& v) {
        // Promote int to float if either operand is a float
        if (v.isFloat())
            return std::get<double>(v.data);
        if (v.isInt())
            return static_cast<double>(std::get<int64_t>(v.data));
        throw std::runtime_error("Cannot convert to float");
    }

    static std::string toString(const Value& v) {
        return std::visit([]<typename T>(const T& x) -> std::string {
            using ValType = std::decay_t<T>;
            if constexpr (std::is_same_v<ValType, NoneType>)
                return "None";
            if constexpr (std::is_same_v<ValType, bool>)
                return x ? "True" : "False";
            if constexpr (std::is_same_v<ValType, int64_t>)
                return std::to_string(x);
            if constexpr (std::is_same_v<ValType, double>)
                return std::to_string(x);
            if constexpr (std::is_same_v<ValType, std::string>)
                return x;
            if constexpr (std::is_same_v<ValType, Value::Fn>)
                return "<builtin>";
            return "<unknown>";
        }, v.data);
    }

    Value add(const Value& lhs, const Value& rhs) {
        if (lhs.isInt() && rhs.isInt())
            return Value(std::get<int64_t>(lhs.data) + std::get<int64_t>(rhs.data));
        if ((lhs.isInt() || lhs.isFloat()) && (rhs.isInt() || rhs.isFloat()))
            return Value(toFloat(lhs) + toFloat(rhs));
        if (lhs.isStr() && rhs.isStr())
            return Value(std::get<std::string>(lhs.data) + std::get<std::string>(rhs.data));
        throw std::runtime_error("Unsupported operand types for +");
    }

    Value sub(const Value& lhs, const Value& rhs) {
        if (lhs.isInt() && rhs.isInt())
            return Value(std::get<int64_t>(lhs.data) - std::get<int64_t>(rhs.data));
        if ((lhs.isInt() || lhs.isFloat()) && (rhs.isInt() || rhs.isFloat()))
            return Value(toFloat(lhs) - toFloat(rhs));
        throw std::runtime_error("Unsupported operand types for -");
    }

    Value mul(const Value& lhs, const Value& rhs) {
        if (lhs.isInt() && rhs.isInt())
            return Value(std::get<int64_t>(lhs.data) * std::get<int64_t>(rhs.data));
        if ((lhs.isInt() || lhs.isFloat()) && (rhs.isInt() || rhs.isFloat()))
            return Value(toFloat(lhs) * toFloat(rhs));
        throw std::runtime_error("Unsupported operand types for *");
    }

    Value div(const Value& lhs, const Value& rhs) {
        // Python's / always returns float
        return Value(toFloat(lhs) / toFloat(rhs));
    }

    Value eq(const Value& lhs, const Value& rhs) {
        return std::visit([]<typename T0, typename T1>(const T0& l, const T1& r) -> Value {
            using L = std::decay_t<T0>;
            using R = std::decay_t<T1>;
            if constexpr (std::is_same_v<L, R>)
                return Value(l == r);
            return Value(false);
        }, lhs.data, rhs.data);
    }

    Value ne(const Value& lhs, const Value& rhs) {
        return Value(!std::get<bool>(eq(lhs, rhs).data));
    }

    Value lt(const Value& lhs, const Value& rhs) {
        if (lhs.isInt() && rhs.isInt())
            return Value(std::get<int64_t>(lhs.data) < std::get<int64_t>(rhs.data));
        if ((lhs.isInt() || lhs.isFloat()) && (rhs.isInt() || rhs.isFloat()))
            return Value(toFloat(lhs) < toFloat(rhs));
        throw std::runtime_error("Unsupported operand types for <");
    }

    Value le(const Value& lhs, const Value& rhs) {
        if (lhs.isInt() && rhs.isInt())
            return Value(std::get<int64_t>(lhs.data) <= std::get<int64_t>(rhs.data));
        if ((lhs.isInt() || lhs.isFloat()) && (rhs.isInt() || rhs.isFloat()))
            return Value(toFloat(lhs) <= toFloat(rhs));
        throw std::runtime_error("Unsupported operand types for <=");
    }

    Value gt(const Value& lhs, const Value& rhs) { return lt(rhs, lhs); }
    Value ge(const Value& lhs, const Value& rhs) { return le(rhs, lhs); }

    bool toBool(Value val) {
        return std::visit([]<typename T>(const T& x) -> bool {
            using ValType = std::decay_t<T>;
            if constexpr (std::is_same_v<ValType, NoneType>)
                return false;
            if constexpr (std::is_same_v<ValType, bool>)
                return x;
            if constexpr (std::is_same_v<ValType, int64_t>)
                return x != 0;
            if constexpr (std::is_same_v<ValType, double>)
                return x != 0.0;
            if constexpr (std::is_same_v<ValType, std::string>)
                return !x.empty();
            if constexpr (std::is_same_v<ValType, Value::Fn>)
                return true;
            return false;
        }, val.data);
    }

    Value builtinPrint(Value* args, const int64_t argc) {
        for (int64_t i = 0; i < argc; i++) {
            if (i > 0)
                printf(" ");
            printf("%s", toString(args[i]).c_str());
        }
        printf("\n");
        return Value(NoneType{});
    }

    Value builtinLen(Value* args, const int64_t argc) {
        if (argc != 1)
            throw std::runtime_error("Too many arguments for len()");
        if (args[0].isStr())
            return Value(static_cast<int64_t>(std::get<std::string>(args[0].data).size()));
        throw std::runtime_error("object has no len()");
    }

    Value builtinInt(Value* args, const int64_t argc) {
        if (argc != 1)
            throw std::runtime_error("Too many arguments for int()");
        return std::visit([]<typename T>(const T& x) -> Value {
            using ValType = std::decay_t<T>;
            if constexpr (std::is_same_v<ValType, int64_t>)
                return Value(x);
            if constexpr (std::is_same_v<ValType, double>)
                return Value(static_cast<int64_t>(x));
            if constexpr (std::is_same_v<ValType, bool>)
                return Value(static_cast<int64_t>(x));
            if constexpr (std::is_same_v<ValType, std::string>)
                return Value(static_cast<int64_t>(std::stoll(x)));
            throw std::runtime_error("Cannot convert to int()");
        }, args[0].data);
    }

    Value builtinFloat(Value* args, const int64_t argc) {
        if (argc != 1)
            throw std::runtime_error("Too many arguments for float()");
        return std::visit([]<typename T>(const T& x) -> Value {
            using ValType = std::decay_t<T>;
            if constexpr (std::is_same_v<ValType, double>)
                return Value(x);
            if constexpr (std::is_same_v<ValType, int64_t>)
                return Value(static_cast<double>(x));
            if constexpr (std::is_same_v<ValType, bool>)
                return Value(static_cast<double>(x));
            if constexpr (std::is_same_v<ValType, std::string>)
                return Value(std::stod(x));
            throw std::runtime_error("cannot convert to float()");
        }, args[0].data);
    }

    Value builtinStr(Value* args, const int64_t argc) {
        if (argc != 1)
            throw std::runtime_error("Too many arguments for str()");
        return Value(toString(args[0]));
    }

    Value builtinBool(Value* args, const int64_t argc) {
        if (argc != 1)
            throw std::runtime_error("Too many arguments for bool()");
        return Value(toBool(args[0]));
    }

    Value loadName(const char* name) {
        static const std::unordered_map<std::string, Value::Fn> builtins = {
                {"print", builtinPrint},
                {"len", builtinLen},
                {"int", builtinInt},
                {"float", builtinFloat},
                {"str", builtinStr},
                {"bool", builtinBool},
        };
        if (const auto it = builtins.find(name); it != builtins.end())
            return Value(it->second);
        throw std::runtime_error(std::string("name '") + name + "' is not defined");
    }

    Value call(const Value& callee, Value* args, const int64_t argc) {
        if (callee.isFn())
            return std::get<Value::Fn>(callee.data)(args, argc);
        throw std::runtime_error("object is not callable");
    }

} //namespace pyir::runtime
