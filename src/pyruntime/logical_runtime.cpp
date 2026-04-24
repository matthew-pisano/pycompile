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
#include "pyruntime/runtime_errors.h"
#include "pyruntime/runtime_util.h"


/// Format an unsupported operands error.
std::string formatUnsupportedOperands(const std::string& op, const std::string& lhsType, const std::string& rhsType) {
    return std::format("Unsupported operand type(s) for {}: '{}' and '{}'", op, lhsType, rhsType);
}

std::string formatUnsupportedOperands(const std::string& op, const std::string& valType) {
    return std::format("Bad operand type for {}: '{}'", op, valType);
}


bool pyir_isBool(const PyObj* val) { return dynamic_cast<const PyBool*>(val); }
bool pyir_isInt(const PyObj* val) { return dynamic_cast<const PyInt*>(val); }
bool pyir_isFloat(const PyObj* val) { return dynamic_cast<const PyFloat*>(val); }
bool pyir_isStr(const PyObj* val) { return dynamic_cast<const PyStr*>(val); }
bool pyir_isList(const PyObj* val) { return dynamic_cast<const PyList*>(val); }
bool pyir_isSet(const PyObj* val) { return dynamic_cast<const PySet*>(val); }
bool pyir_isDict(const PyObj* val) { return dynamic_cast<const PyDict*>(val); }
bool pyir_isTuple(const PyObj* val) { return dynamic_cast<const PyTuple*>(val); }


int8_t pyir_isTruthy(PyObj* val) {
    const int8_t result = val->isTruthy();
    (void) val->decref();
    return result;
}


PyObj* pyir_add(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = new PyInt(dynamic_cast<PyInt*>(lhs)->data() + dynamic_cast<PyInt*>(rhs)->data());
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(valueToFloat(lhs) + valueToFloat(rhs));
    else if (pyir_isStr(lhs) && pyir_isStr(rhs))
        result = new PyStr(dynamic_cast<PyStr*>(lhs)->data() + dynamic_cast<PyStr*>(rhs)->data());
    else if (pyir_isList(lhs) && pyir_isList(rhs)) {
        result = new PyList({});
        PyList::extend(result, &lhs, 1);
        PyList::extend(result, &rhs, 1);
    } else if (pyir_isTuple(lhs) && pyir_isTuple(rhs)) {
        PyListData lhsVal = dynamic_cast<PyTuple*>(lhs)->data();
        PyListData rhsVal = dynamic_cast<PyTuple*>(rhs)->data();
        PyListData combined;
        combined.insert(combined.end(), lhsVal.begin(), lhsVal.end());
        combined.insert(combined.end(), rhsVal.begin(), rhsVal.end());
        result = new PyTuple(combined);
    }

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("+", lhsType, rhsType));
    return result;
}


PyObj* pyir_sub(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = new PyInt(dynamic_cast<PyInt*>(lhs)->data() - dynamic_cast<PyInt*>(rhs)->data());
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(valueToFloat(lhs) - valueToFloat(rhs));
    else if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        PySetData res = dynamic_cast<PySet*>(lhs)->data();
        const PySetData rSet = dynamic_cast<PySet*>(rhs)->data();
        for (PyObj* lItem : dynamic_cast<PySet*>(lhs)->data()) {
            if (rSet.contains(lItem))
                res.erase(lItem);
            else
                lItem->incref();
        }
        result = new PySet(res);
    }

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("-", lhsType, rhsType));
    return result;
}


PyObj* pyir_mul(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = new PyInt(dynamic_cast<PyInt*>(lhs)->data() * dynamic_cast<PyInt*>(rhs)->data());
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(valueToFloat(lhs) * valueToFloat(rhs));

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("*", lhsType, rhsType));
    return result;
}


PyFloat* pyir_div(PyObj* lhs, PyObj* rhs) {
    // Python's / always returns float
    PyFloat* result = nullptr;
    if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(valueToFloat(lhs) / valueToFloat(rhs));

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("/", lhsType, rhsType));
    return result;
}


PyObj* pyir_floorDiv(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = new PyInt(dynamic_cast<PyInt*>(lhs)->data() / dynamic_cast<PyInt*>(rhs)->data());
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(std::floor(valueToFloat(lhs) / valueToFloat(rhs)));

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("//", lhsType, rhsType));
    return result;
}


PyObj* pyir_exp(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs)) {
        const double_t lhsFloat = static_cast<double_t>(dynamic_cast<PyInt*>(lhs)->data());
        const double_t rhsFloat = static_cast<double_t>(dynamic_cast<PyInt*>(rhs)->data());
        result = new PyInt(static_cast<int64_t>(pow(lhsFloat, rhsFloat)));
    } else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(pow(valueToFloat(lhs), valueToFloat(rhs)));

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("**", lhsType, rhsType));
    return result;
}


PyObj* pyir_mod(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = new PyInt(dynamic_cast<PyInt*>(lhs)->data() % dynamic_cast<PyInt*>(rhs)->data());
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = new PyFloat(fmod(valueToFloat(lhs), valueToFloat(rhs)));

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("%", lhsType, rhsType));
    return result;
}


PyObj* pyir_pipe(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        result = new PySet({});
        PySet::update(result, &lhs, 1);
        PySet::update(result, &rhs, 1);
    } else if (pyir_isDict(lhs) && pyir_isDict(rhs)) {
        result = new PyDict({});
        PyDict::update(result, &lhs, 1);
        PyDict::update(result, &rhs, 1);
    }

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("|", lhsType, rhsType));
    return result;
}


PyObj* pyir_ampersand(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        result = new PySet({});
        const PySetData rSet = dynamic_cast<PySet*>(rhs)->data();
        for (PyObj* lItem : dynamic_cast<PySet*>(lhs)->data())
            if (rSet.contains(lItem))
                PySet::add(result, &lItem, 1);
    }

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("&", lhsType, rhsType));
    return result;
}


PyObj* pyir_idx(PyObj* obj, PyObj* idx) {
    try {
        PyObj* result = obj->idx(idx);
        (void) idx->decref();
        (void) obj->decref();
        return result;
    } catch (...) {
        (void) idx->decref();
        (void) obj->decref();
        throw;
    }
}


PyBool* pyir_in(PyObj* container, PyObj* element) {
    PyBool* result = container->contains(element);
    (void) element->decref();
    (void) container->decref();
    return result;
}


PyBool* pyir_eq(PyObj* lhs, PyObj* rhs) {
    PyBool* result = *lhs == *rhs ? PyBool::True : PyBool::False;
    (void) lhs->decref();
    (void) rhs->decref();
    return result;
}


PyBool* pyir_ne(PyObj* lhs, PyObj* rhs) {
    PyBool* result = *lhs != *rhs ? PyBool::True : PyBool::False;
    (void) lhs->decref();
    (void) rhs->decref();
    return result;
}


PyBool* pyir_lt(PyObj* lhs, PyObj* rhs) {
    PyBool* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = dynamic_cast<PyInt*>(lhs)->data() < dynamic_cast<PyInt*>(rhs)->data() ? PyBool::True : PyBool::False;
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = valueToFloat(lhs) < valueToFloat(rhs) ? PyBool::True : PyBool::False;

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("<", lhsType, rhsType));
    return result;
}


PyBool* pyir_le(PyObj* lhs, PyObj* rhs) {
    PyBool* result = nullptr;
    if (pyir_isInt(lhs) && pyir_isInt(rhs))
        result = dynamic_cast<PyInt*>(lhs)->data() <= dynamic_cast<PyInt*>(rhs)->data() ? PyBool::True : PyBool::False;
    else if ((pyir_isFloat(lhs) || pyir_isInt(lhs)) && (pyir_isFloat(rhs) || pyir_isInt(rhs)))
        result = valueToFloat(lhs) <= valueToFloat(rhs) ? PyBool::True : PyBool::False;

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("<=", lhsType, rhsType));
    return result;
}


PyBool* pyir_gt(PyObj* lhs, PyObj* rhs) { return pyir_lt(rhs, lhs); }


PyBool* pyir_ge(PyObj* lhs, PyObj* rhs) { return pyir_le(rhs, lhs); }


PyObj* pyir_unaryNegative(PyObj* val) {
    PyObj* result = nullptr;
    if (pyir_isInt(val))
        result = new PyInt(-dynamic_cast<PyInt*>(val)->data());
    else if (pyir_isFloat(val))
        result = new PyFloat(-dynamic_cast<PyFloat*>(val)->data());

    const std::string valType = val->typeName();
    (void) val->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("-", valType));
    return result;
}


PyObj* pyir_unaryNot(PyObj* val) {
    PyObj* result = nullptr;
    if (pyir_isBool(val))
        result = !dynamic_cast<PyBool*>(val)->data() ? PyBool::True : PyBool::False;

    const std::string valType = val->typeName();
    (void) val->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("not", valType));
    return result;
}


PyInt* pyir_unaryInvert(PyObj* val) {
    PyInt* result = nullptr;
    if (pyir_isInt(val))
        result = new PyInt(~dynamic_cast<PyInt*>(val)->data());

    const std::string valType = val->typeName();
    (void) val->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("~", valType));
    return result;
}


PyObj* pyir_xor(PyObj* lhs, PyObj* rhs) {
    PyObj* result = nullptr;
    if (pyir_isBool(lhs) && pyir_isBool(rhs))
        result = (dynamic_cast<PyBool*>(lhs)->data() ^ dynamic_cast<PyBool*>(rhs)->data()) == 1 ? PyBool::True
                                                                                                : PyBool::False;
    else if (pyir_isSet(lhs) && pyir_isSet(rhs)) {
        const PySetData lSet = dynamic_cast<PySet*>(lhs)->data();
        const PySetData rSet = dynamic_cast<PySet*>(rhs)->data();
        PySetData res;

        // Elements in lSet not in rSet
        for (PyObj* elem : lSet)
            if (!rSet.contains(elem)) {
                res.insert(elem);
                elem->incref();
            }

        // Elements in rSet not in lSet
        for (PyObj* elem : rSet)
            if (!lSet.contains(elem)) {
                res.insert(elem);
                elem->incref();
            }
        result = new PySet(res);
    }

    const std::string lhsType = lhs->typeName();
    const std::string rhsType = rhs->typeName();
    (void) lhs->decref();
    (void) rhs->decref();

    if (!result)
        throw PyTypeError(formatUnsupportedOperands("^", lhsType, rhsType));
    return result;
}


PyBool* pyir_toBool(PyObj* val) {
    PyBool* result = val->isTruthy() ? PyBool::True : PyBool::False;
    (void) val->decref();
    return result;
}


PyStr* pyir_formatSimple(PyObj* val) {
    PyStr* result = new PyStr(valueToString(val));
    (void) val->decref();
    return result;
}
