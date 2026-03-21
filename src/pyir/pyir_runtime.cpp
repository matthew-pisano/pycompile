//
// Created by matthew on 3/8/26.
//

#include "pyir/pyir_runtime.h"

#include <csignal>
#include <format>
#include <stdexcept>
#include <unordered_map>


extern "C" {
static const std::unordered_map<std::string, Value::Fn> builtins = {
        {"print", pyir_builtinPrint},
        {"len", pyir_builtinLen},
        {"int", pyir_builtinInt},
        {"float", pyir_builtinFloat},
        {"str", pyir_builtinStr},
        {"bool", pyir_builtinBool},
};

static std::unordered_map<std::string, Value*> moduleScope;

static double_t toFloat(const Value* v) {
    // Promote int to float if either operand is a float
    if (v->isFloat())
        return std::get<double_t>(v->data);
    if (v->isInt())
        return static_cast<double_t>(std::get<int64_t>(v->data));
    throw std::runtime_error("Cannot convert to float");
}

static std::string toString(const Value* v) {
    return std::visit([]<typename T>(const T& x) -> std::string {
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
    }, v->data);
}

static bool toBool(const Value* val) {
    return std::visit([]<typename T>(const T& x) -> bool {
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
    }, val->data);
}

int8_t pyir_is_truthy(const Value* val) {
    return toBool(val) ? 1 : 0;
}

Value* pyir_add(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) + std::get<int64_t>(rhs->data));
    if ((lhs->isFloat() || lhs->isInt()) && (rhs->isFloat() || rhs->isInt())) {
        const ValueRef l(new Value(toFloat(lhs))); // Temporary promoted float
        const ValueRef r(new Value(toFloat(rhs))); // If this throws, l is cleaned up
        return new Value(std::get<double_t>(l.get()->data) + std::get<double_t>(r.get()->data));
    }
    if (lhs->isStr() && rhs->isStr())
        return new Value(std::get<std::string>(lhs->data) + std::get<std::string>(rhs->data));
    throw std::runtime_error("Unsupported operand types for +");
}

Value* pyir_sub(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) - std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(toFloat(lhs) - toFloat(rhs));
    throw std::runtime_error("Unsupported operand types for -");
}

Value* pyir_mul(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(toFloat(lhs) * toFloat(rhs));
    throw std::runtime_error("Unsupported operand types for *");
}

Value* pyir_div(const Value* lhs, const Value* rhs) {
    // Python's / always returns float
    return new Value(toFloat(lhs) / toFloat(rhs));
}

Value* pyir_eq(const Value* lhs, const Value* rhs) {
    return std::visit([]<typename T0, typename T1>(const T0& l, const T1& r) -> Value* {
        using L = std::decay_t<T0>;
        using R = std::decay_t<T1>;
        if constexpr (std::is_same_v<L, R>)
            return new Value(l == r);
        return new Value(false);
    }, lhs->data, rhs->data);
}

Value* pyir_ne(const Value* lhs, const Value* rhs) {
    return new Value(!std::get<bool>(pyir_eq(lhs, rhs)->data));
}

Value* pyir_lt(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) < std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(toFloat(lhs) < toFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <");
}

Value* pyir_le(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) <= std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(toFloat(lhs) <= toFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <=");
}

Value* pyir_gt(const Value* lhs, const Value* rhs) { return pyir_lt(rhs, lhs); }
Value* pyir_ge(const Value* lhs, const Value* rhs) { return pyir_le(rhs, lhs); }

Value* pyir_unary_negative(const Value* val) {
    if (val->isInt())
        return new Value(-std::get<int64_t>(val->data));
    if (val->isFloat())
        return new Value(-std::get<double_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary -");
}

Value* pyir_unary_not(const Value* val) {
    if (val->isBool())
        return new Value(!std::get<bool>(val->data));
    throw std::runtime_error("Unsupported operand type for unary not");
}

Value* pyir_unary_invert(const Value* val) {
    if (val->isInt())
        return new Value(~std::get<int64_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary ~");
}

Value* pyir_xor(const Value* lhs, const Value* rhs) {
    if (lhs->isBool() && rhs->isBool())
        return new Value(static_cast<int64_t>(std::get<bool>(lhs->data) ^ std::get<bool>(rhs->data)));
    throw std::runtime_error("Unsupported operand type for unary not");
}

Value* pyir_builtinPrint(Value** args, const int64_t argc) {
    for (int64_t i = 0; i < argc; i++) {
        if (i > 0)
            printf(" ");
        printf("%s", toString(args[i]).c_str());
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
    return new Value(toString(args[0]));
}

Value* pyir_builtinBool(Value** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for bool()");
    return new Value(toBool(args[0]));
}

Value* pyir_to_bool(const Value* val) {
    return new Value(toBool(val));
}

Value* pyir_load_name(const char* name) {
    // Check for builtins
    if (const auto it = builtins.find(name); it != builtins.end())
        return new Value(it->second);
    // Check for names in module scope
    if (const auto it = moduleScope.find(name); it != moduleScope.end()) {
        it->second->incref();
        return it->second;
    }
    throw std::runtime_error(std::string("name '") + name + "' is not defined");
}

void pyir_store_name(const char* name, Value* val) {
    if (const auto it = moduleScope.find(name); it != moduleScope.end())
        it->second->decref(); // Release old value
    val->incref();
    moduleScope[name] = val;
}

Value* pyir_load_const_str(const char* str) {
    return new Value(std::string(str));
}

Value* pyir_load_const_int(const int64_t val) {
    return new Value(val);
}

Value* pyir_load_const_float(const double_t val) {
    return new Value(val);
}

Value* pyir_load_const_bool(const int8_t val) {
    return new Value(val == 1);
}

Value* pyir_call(const Value* callee, Value** args, const int64_t argc) {
    if (!callee->isFn())
        throw std::runtime_error("object is not callable");
    // If the fn throws, result never gets returned, leak if anything was allocated
    ValueRef result(std::get<Value::Fn>(callee->data)(args, argc));
    return result.release();
}

void pyir_decref(Value* v) {
    if (v)
        v->decref();
}

Value* pyir_push_null() {
    return nullptr;
}

}
