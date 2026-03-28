//
// Created by matthew on 3/24/26.
//

#include "pyruntime/logical_runtime.h"

#include <cmath>
#include <stdexcept>

#include "pyruntime/runtime_util.h"


int8_t pyir_isTruthy(const Value* val) { return valueToBool(val) ? 1 : 0; }


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
    if (lhs->isList() && rhs->isList()) {
        Value::List lhsVal = std::get<Value::List>(lhs->data);
        Value::List rhsVal = std::get<Value::List>(rhs->data);
        Value::List result;
        result.insert(result.end(), lhsVal.begin(), lhsVal.end());
        result.insert(result.end(), rhsVal.begin(), rhsVal.end());
        return new Value(result);
    }
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


Value* pyir_idx(const Value* obj, const Value* idx) {
    if (!idx->isInt())
        throw std::runtime_error("List indices must be integers");

    if (obj->isStr()) {
        const std::string& str = std::get<std::string>(obj->data);
        int64_t index = std::get<int64_t>(idx->data);
        if (index < 0)
            index += str.size();
        if (index < 0 || static_cast<size_t>(index) >= str.size())
            throw std::runtime_error("String index out of range");
        return new Value(std::string(1, str[index]));
    }
    if (obj->isList()) {
        const Value::List& list = std::get<Value::List>(obj->data);
        int64_t index = std::get<int64_t>(idx->data);
        if (index < 0)
            index += list.size();
        if (index < 0 || static_cast<size_t>(index) >= list.size())
            throw std::runtime_error("List index out of range");
        list[index]->incref(); // Return a new reference to the indexed value
        return list[index];
    }

    throw std::runtime_error("Unsupported operand type for indexing");
}


Value* pyir_eq(const Value* lhs, const Value* rhs) {
    return std::visit(
            []<typename T0, typename T1>(const T0& l, const T1& r) -> Value* {
                using L = std::decay_t<T0>;
                using R = std::decay_t<T1>;
                if constexpr (std::is_same_v<L, R>)
                    return new Value(l == r);
                return new Value(false);
            },
            lhs->data, rhs->data);
}


Value* pyir_ne(const Value* lhs, const Value* rhs) { return new Value(!std::get<bool>(pyir_eq(lhs, rhs)->data)); }


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


Value* pyir_gt(const Value* lhs, const Value* rhs) { return pyir_lt(rhs, lhs); }


Value* pyir_ge(const Value* lhs, const Value* rhs) { return pyir_le(rhs, lhs); }


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


Value* pyir_toBool(const Value* val) { return new Value(valueToBool(val)); }


Value* pyir_formatSimple(const Value* val) { return new Value(valueToString(val)); }
