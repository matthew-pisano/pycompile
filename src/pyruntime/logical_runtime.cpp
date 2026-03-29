//
// Created by matthew on 3/24/26.
//

#include "pyruntime/logical_runtime.h"

#include <cmath>
#include <stdexcept>

#include "pyruntime/runtime_util.h"


int8_t pyir_isTruthy(const PyValue* val) { return valueToBool(val) ? 1 : 0; }


PyValue* pyir_add(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) + std::get<int64_t>(rhs->data));
    if ((lhs->isFloat() || lhs->isInt()) && (rhs->isFloat() || rhs->isInt())) {
        const ValueRef l(new PyValue(valueToFloat(lhs))); // Temporary promoted float
        const ValueRef r(new PyValue(valueToFloat(rhs))); // If this throws, l is cleaned up
        return new PyValue(std::get<double_t>(l.get()->data) + std::get<double_t>(r.get()->data));
    }
    if (lhs->isStr() && rhs->isStr())
        return new PyValue(std::get<std::string>(lhs->data) + std::get<std::string>(rhs->data));
    if (lhs->isList() && rhs->isList()) {
        PyList lhsVal = std::get<PyList>(lhs->data);
        PyList rhsVal = std::get<PyList>(rhs->data);
        PyList result;
        result.insert(result.end(), lhsVal.begin(), lhsVal.end());
        result.insert(result.end(), rhsVal.begin(), rhsVal.end());
        return new PyValue(result);
    }
    throw std::runtime_error("Unsupported operand types for +");
}


PyValue* pyir_sub(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) - std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(valueToFloat(lhs) - valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for -");
}


PyValue* pyir_mul(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for *");
}


PyValue* pyir_div(const PyValue* lhs, const PyValue* rhs) {
    // Python's / always returns float
    return new PyValue(valueToFloat(lhs) / valueToFloat(rhs));
}


PyValue* pyir_floorDiv(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) / std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(std::floor(valueToFloat(lhs) / valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for //");
}


PyValue* pyir_exp(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for **");
}


PyValue* pyir_mod(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) % std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(fmod(valueToFloat(lhs), valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for %");
}


PyValue* pyir_idx(const PyValue* obj, const PyValue* idx) {
    if (!idx->isInt())
        throw std::runtime_error("List indices must be integers");

    if (obj->isStr()) {
        const std::string& str = std::get<std::string>(obj->data);
        int64_t index = std::get<int64_t>(idx->data);
        if (index < 0)
            index += str.size();
        if (index < 0 || static_cast<size_t>(index) >= str.size())
            throw std::runtime_error("String index out of range");
        return new PyValue(std::string(1, str[index]));
    }
    if (obj->isList()) {
        const PyList& list = std::get<PyList>(obj->data);
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


PyValue* pyir_eq(const PyValue* lhs, const PyValue* rhs) {
    return std::visit(
            []<typename T0, typename T1>(const T0& l, const T1& r) -> PyValue* {
                using L = std::decay_t<T0>;
                using R = std::decay_t<T1>;
                if constexpr (std::is_same_v<L, R>)
                    return new PyValue(l == r);
                return new PyValue(false);
            },
            lhs->data, rhs->data);
}


PyValue* pyir_ne(const PyValue* lhs, const PyValue* rhs) { return new PyValue(!std::get<bool>(pyir_eq(lhs, rhs)->data)); }


PyValue* pyir_lt(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) < std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(valueToFloat(lhs) < valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <");
}


PyValue* pyir_le(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyValue(std::get<int64_t>(lhs->data) <= std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyValue(valueToFloat(lhs) <= valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <=");
}


PyValue* pyir_gt(const PyValue* lhs, const PyValue* rhs) { return pyir_lt(rhs, lhs); }


PyValue* pyir_ge(const PyValue* lhs, const PyValue* rhs) { return pyir_le(rhs, lhs); }


PyValue* pyir_unaryNegative(const PyValue* val) {
    if (val->isInt())
        return new PyValue(-std::get<int64_t>(val->data));
    if (val->isFloat())
        return new PyValue(-std::get<double_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary -");
}


PyValue* pyir_unaryNot(const PyValue* val) {
    if (val->isBool())
        return new PyValue(!std::get<bool>(val->data));
    throw std::runtime_error("Unsupported operand type for unary not");
}


PyValue* pyir_unaryInvert(const PyValue* val) {
    if (val->isInt())
        return new PyValue(~std::get<int64_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary ~");
}


PyValue* pyir_xor(const PyValue* lhs, const PyValue* rhs) {
    if (lhs->isBool() && rhs->isBool())
        return new PyValue((std::get<bool>(lhs->data) ^ std::get<bool>(rhs->data)) == 1);
    throw std::runtime_error("Unsupported operand type for unary not");
}


PyValue* pyir_toBool(const PyValue* val) { return new PyValue(valueToBool(val)); }


PyValue* pyir_formatSimple(const PyValue* val) { return new PyValue(valueToString(val)); }
