//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_tuple.h"

#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/runtime_util.h"

PyInt* PyTuple::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PyTuple::contains(const PyObj* obj) const {
    for (const PyObj* elem : raw)
        if (*elem == *obj)
            return new PyBool(true);
    return new PyBool(false);
}

size_t PyTuple::hash() const {
    size_t hash = 0;
    for (const PyObj* obj : raw)
        hash ^= obj->hash();
    return hash;
}

std::string PyTuple::toString() const {
    if (raw.empty())
        return "()";

    std::string result = "(";
    for (const PyObj* elem : raw)
        result += valueToString(elem, true) + ", ";
    result.pop_back();
    // Only remove last comma if more than a 1-tuple
    if (raw.size() > 1)
        result.pop_back();
    result += ")";
    return result;
}

std::string PyTuple::typeName() const { return "tuple"; }

bool PyTuple::isTruthy() const { return !raw.empty(); }

PyListData PyTuple::data() const { return raw; }

bool PyTuple::operator==(const PyObj& other) const {
    if (const PyTuple* t = dynamic_cast<const PyTuple*>(&other)) {
        if (raw.size() != t->raw.size())
            return false;

        for (size_t i = 0; i < raw.size(); i++)
            if (*raw[i] != *t->raw[i])
                return false;
        return true;
    }
    return false;
}
