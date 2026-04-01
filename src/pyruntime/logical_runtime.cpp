//
// Created by matthew on 3/24/26.
//

#include "pyruntime/logical_runtime.h"

#include <cmath>
#include <stdexcept>

#include "pyruntime/objects/py_object.h"
#include "pyruntime/runtime_util.h"


int8_t pyir_isTruthy(const PyObj* val) { return val->isTruthy(); }


PyObj* pyir_add(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) + std::get<int64_t>(rhs->data));
    if ((lhs->isFloat() || lhs->isInt()) && (rhs->isFloat() || rhs->isInt())) {
        const ValueRef l(new PyObj(valueToFloat(lhs))); // Temporary promoted float
        const ValueRef r(new PyObj(valueToFloat(rhs))); // If this throws, l is cleaned up
        return new PyObj(std::get<double_t>(l.get()->data) + std::get<double_t>(r.get()->data));
    }
    if (lhs->isStr() && rhs->isStr())
        return new PyObj(std::get<std::string>(lhs->data) + std::get<std::string>(rhs->data));
    if (lhs->isList() && rhs->isList()) {
        PyList lhsVal = std::get<PyList>(lhs->data);
        PyList rhsVal = std::get<PyList>(rhs->data);
        PyList result;
        result.data().insert(result.data().end(), lhsVal.data().begin(), lhsVal.data().end());
        result.data().insert(result.data().end(), rhsVal.data().begin(), rhsVal.data().end());
        return new PyObj(result);
    }
    throw std::runtime_error("Unsupported operand types for +");
}


PyObj* pyir_sub(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) - std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(valueToFloat(lhs) - valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for -");
}


PyObj* pyir_mul(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for *");
}


PyObj* pyir_div(const PyObj* lhs, const PyObj* rhs) {
    // Python's / always returns float
    return new PyObj(valueToFloat(lhs) / valueToFloat(rhs));
}


PyObj* pyir_floorDiv(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) / std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(std::floor(valueToFloat(lhs) / valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for //");
}


PyObj* pyir_exp(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) * std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for **");
}


PyObj* pyir_mod(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) % std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(fmod(valueToFloat(lhs), valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for %");
}


PyObj* pyir_idx(PyObj* obj, const PyObj* idx) {
    if (!idx->isInt())
        throw std::runtime_error("List indices must be integers");

    if (obj->isStr()) {
        const std::string& str = std::get<std::string>(obj->data);
        int64_t index = std::get<int64_t>(idx->data);
        if (index < 0)
            index += str.size();
        if (index < 0 || static_cast<size_t>(index) >= str.size())
            throw std::runtime_error("String index out of range");
        return new PyObj(std::string(1, str[index]));
    }
    if (obj->isList()) {
        PyList& list = std::get<PyList>(obj->data);
        int64_t index = std::get<int64_t>(idx->data);
        if (index < 0)
            index += list.data().size();
        if (index < 0 || static_cast<size_t>(index) >= list.data().size())
            throw std::runtime_error("List index out of range");
        list.data()[index]->incref(); // Return a new reference to the indexed value
        return list.data()[index];
    }

    throw std::runtime_error("Unsupported operand type for indexing");
}


PyObj* pyir_eq(const PyObj* lhs, const PyObj* rhs) {
    return std::visit(
            []<typename T0, typename T1>(const T0& l, const T1& r) -> PyObj* {
                using L = std::decay_t<T0>;
                using R = std::decay_t<T1>;
                if constexpr (std::is_same_v<L, R>)
                    return new PyObj(l == r);
                return new PyObj(false);
            },
            lhs->data, rhs->data);
}


PyObj* pyir_ne(const PyObj* lhs, const PyObj* rhs) { return new PyObj(!std::get<bool>(pyir_eq(lhs, rhs)->data)); }


PyObj* pyir_lt(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) < std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(valueToFloat(lhs) < valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <");
}


PyObj* pyir_le(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isInt() && rhs->isInt())
        return new PyObj(std::get<int64_t>(lhs->data) <= std::get<int64_t>(rhs->data));
    if ((lhs->isInt() || lhs->isFloat()) && (rhs->isInt() || rhs->isFloat()))
        return new PyObj(valueToFloat(lhs) <= valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <=");
}


PyObj* pyir_gt(const PyObj* lhs, const PyObj* rhs) { return pyir_lt(rhs, lhs); }


PyObj* pyir_ge(const PyObj* lhs, const PyObj* rhs) { return pyir_le(rhs, lhs); }


PyObj* pyir_unaryNegative(const PyObj* val) {
    if (val->isInt())
        return new PyObj(-std::get<int64_t>(val->data));
    if (val->isFloat())
        return new PyObj(-std::get<double_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary -");
}


PyObj* pyir_unaryNot(const PyObj* val) {
    if (val->isBool())
        return new PyObj(!std::get<bool>(val->data));
    throw std::runtime_error("Unsupported operand type for unary not");
}


PyObj* pyir_unaryInvert(const PyObj* val) {
    if (val->isInt())
        return new PyObj(~std::get<int64_t>(val->data));
    throw std::runtime_error("Unsupported operand type for unary ~");
}


PyObj* pyir_xor(const PyObj* lhs, const PyObj* rhs) {
    if (lhs->isBool() && rhs->isBool())
        return new PyObj((std::get<bool>(lhs->data) ^ std::get<bool>(rhs->data)) == 1);
    throw std::runtime_error("Unsupported operand type for unary not");
}


PyObj* pyir_toBool(const PyObj* val) { return new PyObj(valueToBool(val)); }


PyObj* pyir_formatSimple(const PyObj* val) { return new PyObj(valueToString(val)); }
