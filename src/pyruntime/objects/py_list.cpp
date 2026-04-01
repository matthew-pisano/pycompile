//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_list.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/runtime_util.h"

PyNone* PyList::append(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("append() takes exactly one argument");
    pyir_listAppend(this, args[0]);
    return new PyNone();
}

PyNone* PyList::extend(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("extend() takes exactly one argument");
    pyir_listExtend(this, args[0]);
    return new PyNone();
}

PyInt* PyList::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

std::string PyList::toString() const {
    if (raw.empty())
        return "[]";

    std::string result = "[";
    for (size_t i = 0; i < raw.size() - 1; i++)
        result += valueToString(raw[i], true) + ", ";
    result += valueToString(raw[raw.size() - 1], true) + "]";
    return result;
}

std::string PyList::typeName() const { return "list"; }

bool PyList::isTruthy() const { return !raw.empty(); }

const std::unordered_map<std::string, PyMethod> PyList::attrs() const { return {}; }

std::vector<PyObj*> PyList::data() const { return raw; }

std::vector<PyObj*>& PyList::data() { return raw; }

bool PyList::operator==(const PyObj& other) const {
    if (const PyList* l = dynamic_cast<const PyList*>(&other))
        return raw == l->data();
    return false;
}
