//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builder_runtime.h"

#include <stdexcept>

#include "pyruntime/runtime_objects.h"


PyValue* pyir_buildString(PyValue** parts, const int64_t count) {
    std::string result;
    for (int64_t i = 0; i < count; i++) {
        if (!parts[i]->isStr())
            throw std::runtime_error("BUILD_STRING: expected string part");
        result += std::get<std::string>(parts[i]->data);
    }
    return new PyValue(result);
}


PyValue* pyir_buildList(PyValue** parts, const int64_t count) {
    PyList result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.data().push_back(parts[i]);
    }
    return new PyValue(result);
}


void pyir_listExtend(PyValue* list, const PyValue* items) {
    if (!list->isList() || !items->isList())
        throw std::runtime_error("Can only extend list types with list types");

    PyList& dest = std::get<PyList>(list->data);
    const PyList& src = std::get<PyList>(items->data);
    for (PyValue* v : src.data()) {
        v->incref();
        dest.data().push_back(v);
    }
}


void pyir_listAppend(PyValue* list, PyValue* item) {
    if (!list->isList())
        throw std::runtime_error("Can only append to list types");

    PyList& dest = std::get<PyList>(list->data);
    item->incref();
    dest.data().push_back(item);
}
