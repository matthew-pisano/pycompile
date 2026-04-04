//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builder_runtime.h"

#include <stdexcept>
#include <unordered_set>

#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_str.h"


PyObj* pyir_buildString(PyObj** parts, const int64_t count) {
    std::string result;
    for (int64_t i = 0; i < count; i++) {
        const PyStr* pyStr = dynamic_cast<PyStr*>(parts[i]);
        if (!pyStr)
            throw std::runtime_error("BUILD_STRING: expected string part");
        result += pyStr->data();
    }
    return new PyStr(result);
}


PyObj* pyir_buildList(PyObj** parts, const int64_t count) {
    std::vector<PyObj*> result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.push_back(parts[i]);
    }
    return new PyList(result);
}


void pyir_listExtend(PyObj* list, const PyObj* items) {
    PyList* dest = dynamic_cast<PyList*>(list);
    const PyList* src = dynamic_cast<const PyList*>(items);
    if (!dest || !src)
        throw std::runtime_error("Can only extend list types with list types");

    for (PyObj* v : src->data()) {
        v->incref();
        dest->data().push_back(v);
    }
}


void pyir_listAppend(PyObj* list, PyObj* item) {
    PyList* dest = dynamic_cast<PyList*>(list);
    if (!dest)
        throw std::runtime_error("Can only append to list types");

    item->incref();
    dest->data().push_back(item);
}


PyObj* pyir_buildSet(PyObj** parts, const int64_t count) {
    std::unordered_set<PyObj*> result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.insert(parts[i]);
    }
    return new PySet(result);
}


void pyir_setUpdate(PyObj* list, const PyObj* items) {
    PySet* dest = dynamic_cast<PySet*>(list);
    const PySet* src = dynamic_cast<const PySet*>(items);
    if (!dest || !src)
        throw std::runtime_error("Can only extend list types with list types");

    for (PyObj* v : src->data()) {
        v->incref();
        dest->data().insert(v);
    }
}


void pyir_setAdd(PyObj* list, PyObj* item) {
    PySet* dest = dynamic_cast<PySet*>(list);
    if (!dest)
        throw std::runtime_error("Can only append to list types");

    item->incref();
    dest->data().insert(item);
}
