//
// Created by matthew on 3/24/26.
//

#include "pyruntime/logical_runtime.h"

#include <cmath>
#include <stdexcept>

#include "pyruntime/runtime_util.h"


int8_t pyir_isTruthy(const Value* val) {
    return valueToBool(val) ? 1 : 0;
}


Value* pyir_add(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) + std::get<int64_t>(rhs->data));
    if ((lhs->isFloat() || lhs->isInt()) && (rhs->isFloat() || rhs->isInt())) {
        const ValueRef l(new Value(valueToFloat(lhs))); // Temporary promoted float
        const ValueRef r(new Value(valueToFloat(rhs))); // If this throws, l is cleaned up
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
        return new Value(valueToFloat(lhs) - valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for -");
}


Value* pyir_mul(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for *");
}


Value* pyir_div(const Value* lhs, const Value* rhs) {
    // Python's / always returns float
    return new Value(valueToFloat(lhs) / valueToFloat(rhs));
}


Value* pyir_floorDiv(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) / std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(std::floor(valueToFloat(lhs) / valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for //");
}


Value* pyir_exp(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for **");
}


Value* pyir_mod(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) % std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(fmod(valueToFloat(lhs), valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for %");
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
        return new Value(valueToFloat(lhs) < valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <");
}


Value* pyir_le(const Value* lhs, const Value* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new Value(std::get<int64_t>(lhs->data) <= std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new Value(valueToFloat(lhs) <= valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <=");
}


Value* pyir_gt(const Value* lhs, const Value* rhs) {
    return pyir_lt(rhs, lhs);
}


Value* pyir_ge(const Value* lhs, const Value* rhs) {
    return pyir_le(rhs, lhs);
}


Value* pyir_unaryNegative(const Value* val) {
    if (val->isInt())
        return new Value(-std::get<int64_t>(val->data));
    if (val->isFloat())
        return new Value(-std::get<double_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary -");
}


Value* pyir_unaryNot(const Value* val) {
    if (val->isBool())
        return new Value(!std::get<bool>(val->data));
    throw std::runtime_error("Unsupported operand type for unary not");
}


Value* pyir_unaryInvert(const Value* val) {
    if (val->isInt())
        return new Value(~std::get<int64_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary ~");
}


Value* pyir_xor(const Value* lhs, const Value* rhs) {
    if (lhs->isBool() && rhs->isBool())
        return new Value((std::get<bool>(lhs->data) ^ std::get<bool>(rhs->data)) == 1);
    throw std::runtime_error("Unsupported operand type for unary not");
}


Value* pyir_toBool(const Value* val) {
    return new Value(valueToBool(val));
}


Value* pyir_formatSimple(const Value* val) {
    return new Value(valueToString(val));
}
