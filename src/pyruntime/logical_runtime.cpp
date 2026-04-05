//
// Created by matthew on 3/24/26.
//

#include "pyruntime/logical_runtime.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_object.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_util.h"


bool pyir_isBool(const PyObj* val) { return dynamic_cast<const PyBool*>(val); }
bool pyir_isInt(const PyObj* val) { return dynamic_cast<const PyInt*>(val); }
bool pyir_isFloat(const PyObj* val) { return dynamic_cast<const PyFloat*>(val); }
bool pyir_isStr(const PyObj* val) { return dynamic_cast<const PyStr*>(val); }
bool pyir_isList(const PyObj* val) { return dynamic_cast<const PyList*>(val); }
bool pyir_isSet(const PyObj* val) { return dynamic_cast<const PySet*>(val); }


int8_t pyir_isTruthy(const PyObj* val) { return val->isTruthy(); }


PyObj* pyir_add(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyInt(dynamic_cast<const PyInt*>(lhs)->data() + dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyFloat(valueToFloat(lhs) + valueToFloat(rhs));
    if (pyir_isStr(lhs) && pyir_isStr(rhs))
        return new PyStr(dynamic_cast<const PyStr*>(lhs)->data() + dynamic_cast<const PyStr*>(rhs)->data());
    if (pyir_isList(lhs) && pyir_isList(rhs)) {
        PyListData lhsVal = dynamic_cast<const PyList*>(lhs)->data();
        PyListData rhsVal = dynamic_cast<const PyList*>(rhs)->data();
        PyListData result;
        result.insert(result.end(), lhsVal.begin(), lhsVal.end());
        result.insert(result.end(), rhsVal.begin(), rhsVal.end());
        return new PyList(result);
    }
    throw std::runtime_error("Unsupported operand types for +");
}


PyObj* pyir_sub(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyInt(dynamic_cast<const PyInt*>(lhs)->data() - dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyFloat(valueToFloat(lhs) - valueToFloat(rhs));
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        PySetData result = dynamic_cast<const PySet*>(lhs)->data();
        const PySetData rSet = dynamic_cast<const PySet*>(rhs)->data();
        for (PyObj* lItem : dynamic_cast<const PySet*>(lhs)->data())
            if (unorderedSetContains(rSet, lItem))
                result.erase(lItem);

        return new PySet(result);
    }
    throw std::runtime_error("Unsupported operand types for -");
}


PyObj* pyir_mul(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyInt(dynamic_cast<const PyInt*>(lhs)->data() * dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyFloat(valueToFloat(lhs) * valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for *");
}


PyFloat* pyir_div(const PyObj* lhs, const PyObj* rhs) {
    // Python's / always returns float
    return new PyFloat(valueToFloat(lhs) / valueToFloat(rhs));
}


PyObj* pyir_floorDiv(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyInt(dynamic_cast<const PyInt*>(lhs)->data() / dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyFloat(std::floor(valueToFloat(lhs) / valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for //");
}


PyObj* pyir_exp(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyInt(pow(dynamic_cast<const PyInt*>(lhs)->data(), dynamic_cast<const PyInt*>(rhs)->data()));
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyFloat(pow(valueToFloat(lhs), valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for **");
}


PyObj* pyir_mod(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyInt(dynamic_cast<const PyInt*>(lhs)->data() % dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyFloat(fmod(valueToFloat(lhs), valueToFloat(rhs)));
    throw std::runtime_error("Unsupported operand types for %");
}


PyObj* pyir_pipe(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        PySetData result = dynamic_cast<const PySet*>(lhs)->data();
        const PySetData rhsSet = dynamic_cast<const PySet*>(rhs)->data();
        result.insert(rhsSet.begin(), rhsSet.end());
        return new PySet(result);
    }
    throw std::runtime_error("Unsupported operand types for |");
}


PyObj* pyir_ampersand(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        PySetData result;
        const PySetData rSet = dynamic_cast<const PySet*>(rhs)->data();
        for (PyObj* lItem : dynamic_cast<const PySet*>(lhs)->data())
            if (unorderedSetContains(rSet, lItem))
                result.insert(lItem);
        return new PySet(result);
    }
    throw std::runtime_error("Unsupported operand types for &");
}


PyObj* pyir_idx(const PyObj* obj, const PyObj* idx) {
    if (!pyir_isInt(idx))
        throw std::runtime_error("List indices must be integers");

    int64_t index = dynamic_cast<const PyInt*>(idx)->data();
    if (pyir_isStr(obj)) {
        const std::string& str = dynamic_cast<const PyStr*>(obj)->data();
        if (index < 0)
            index += static_cast<int64_t>(str.size());
        if (index < 0 || index >= static_cast<int64_t>(str.size()))
            throw std::runtime_error("String index out of range");
        return new PyStr(str[index]);
    }
    if (pyir_isList(obj)) {
        const PyListData& list = dynamic_cast<const PyList*>(obj)->data();
        if (index < 0)
            index += static_cast<int64_t>(list.size());
        if (index < 0 || index >= static_cast<int64_t>(list.size()))
            throw std::runtime_error("List index out of range");
        list[index]->incref(); // Return a new reference to the indexed value
        return list[index];
    }

    throw std::runtime_error("Unsupported operand type for indexing");
}


PyBool* pyir_in(const PyObj* container, const PyObj* element) {
    if (const PyList* list = dynamic_cast<const PyList*>(container)) {
        for (const PyObj* obj : list->data())
            if (*obj == *element)
                return new PyBool(true);
    } else
        throw std::runtime_error("Unsupported operand types for in");

    // Operation valid, but element not found for any path
    return new PyBool(false);
}


PyBool* pyir_eq(const PyObj* lhs, const PyObj* rhs) { return new PyBool(*lhs == *rhs); }


PyBool* pyir_ne(const PyObj* lhs, const PyObj* rhs) { return new PyBool(!pyir_eq(lhs, rhs)->data()); }


PyBool* pyir_lt(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyBool(dynamic_cast<const PyInt*>(lhs)->data() < dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyBool(valueToFloat(lhs) < valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <");
}


PyBool* pyir_le(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        return new PyBool(dynamic_cast<const PyInt*>(lhs)->data() <= dynamic_cast<const PyInt*>(rhs)->data());
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        return new PyBool(valueToFloat(lhs) <= valueToFloat(rhs));
    throw std::runtime_error("Unsupported operand types for <=");
}


PyBool* pyir_gt(const PyObj* lhs, const PyObj* rhs) { return pyir_lt(rhs, lhs); }


PyBool* pyir_ge(const PyObj* lhs, const PyObj* rhs) { return pyir_le(rhs, lhs); }


PyObj* pyir_unaryNegative(const PyObj* val) {
    if (pyir_isInt(val))
        return new PyInt(-dynamic_cast<const PyInt*>(val)->data());
    if (pyir_isFloat(val))
        return new PyFloat(-dynamic_cast<const PyFloat*>(val)->data());
    throw std::runtime_error("Unsupported operand type for unary -");
}


PyObj* pyir_unaryNot(const PyObj* val) {
    if (pyir_isBool(val))
        return new PyBool(!dynamic_cast<const PyBool*>(val)->data());
    throw std::runtime_error("Unsupported operand type for unary not");
}


PyInt* pyir_unaryInvert(const PyObj* val) {
    if (pyir_isInt(val))
        return new PyInt(~dynamic_cast<const PyInt*>(val)->data());
    throw std::runtime_error("Unsupported operand type for unary ~");
}


PyObj* pyir_xor(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isBool(lhs) && pyir_isBool(rhs))
        return new PyBool((dynamic_cast<const PyBool*>(lhs)->data() ^ dynamic_cast<const PyBool*>(rhs)->data()) == 1);
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        const PySetData lSet = dynamic_cast<const PySet*>(lhs)->data();
        const PySetData rSet = dynamic_cast<const PySet*>(rhs)->data();
        PySetData result;

        // Elements in lSet not in rSet
        for (PyObj* elem : lSet)
            if (!unorderedSetContains(rSet, elem))
                result.insert(elem);

        // Elements in rSet not in lSet
        for (PyObj* elem : rSet)
            if (!unorderedSetContains(lSet, elem))
                result.insert(elem);

        return new PySet(result);
    }
    throw std::runtime_error("Unsupported operand type for ^");
}


PyBool* pyir_toBool(const PyObj* val) { return new PyBool(val->isTruthy()); }


PyStr* pyir_formatSimple(const PyObj* val) { return new PyStr(valueToString(val)); }
