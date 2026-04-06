//
// Created by matthew on 3/24/26.
//

#include "pyruntime/logical_runtime.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_dict.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_object.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/objects/py_tuple.h"
#include "pyruntime/runtime_util.h"


bool pyir_isBool(const PyObj* val) { return dynamic_cast<const PyBool*>(val); }
bool pyir_isInt(const PyObj* val) { return dynamic_cast<const PyInt*>(val); }
bool pyir_isFloat(const PyObj* val) { return dynamic_cast<const PyFloat*>(val); }
bool pyir_isStr(const PyObj* val) { return dynamic_cast<const PyStr*>(val); }
bool pyir_isList(const PyObj* val) { return dynamic_cast<const PyList*>(val); }
bool pyir_isSet(const PyObj* val) { return dynamic_cast<const PySet*>(val); }
bool pyir_isDict(const PyObj* val) { return dynamic_cast<const PyDict*>(val); }
bool pyir_isTuple(const PyObj* val) { return dynamic_cast<const PyTuple*>(val); }


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
    if (pyir_isTuple(lhs) && pyir_isTuple(rhs)) {
        PyListData lhsVal = dynamic_cast<const PyTuple*>(lhs)->data();
        PyListData rhsVal = dynamic_cast<const PyTuple*>(rhs)->data();
        PyListData result;
        result.insert(result.end(), lhsVal.begin(), lhsVal.end());
        result.insert(result.end(), rhsVal.begin(), rhsVal.end());
        return new PyTuple(result);
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
            if (rSet.contains(lItem))
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
    if (pyir_isDict(lhs) && pyir_isDict(rhs)) {
        PyDictData result = dynamic_cast<const PyDict*>(lhs)->data();
        const PyDictData rhsDict = dynamic_cast<const PyDict*>(rhs)->data();
        result.insert(rhsDict.begin(), rhsDict.end());
        return new PyDict(result);
    }
    throw std::runtime_error("Unsupported operand types for |");
}


PyObj* pyir_ampersand(const PyObj* lhs, const PyObj* rhs) {
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        PySetData result;
        const PySetData rSet = dynamic_cast<const PySet*>(rhs)->data();
        for (PyObj* lItem : dynamic_cast<const PySet*>(lhs)->data())
            if (rSet.contains(lItem))
                result.insert(lItem);
        return new PySet(result);
    }
    throw std::runtime_error("Unsupported operand types for &");
}


PyObj* pyir_idx(const PyObj* obj, const PyObj* idx) { return obj->idx(idx); }


PyBool* pyir_in(const PyObj* container, const PyObj* element) { return container->contains(element); }


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
            if (!rSet.contains(elem))
                result.insert(elem);

        // Elements in rSet not in lSet
        for (PyObj* elem : rSet)
            if (!lSet.contains(elem))
                result.insert(elem);

        return new PySet(result);
    }
    throw std::runtime_error("Unsupported operand type for ^");
}


PyBool* pyir_toBool(const PyObj* val) { return new PyBool(val->isTruthy()); }


PyStr* pyir_formatSimple(const PyObj* val) { return new PyStr(valueToString(val)); }
