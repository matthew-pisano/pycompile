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
    PyListData result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.push_back(parts[i]);
    }
    return new PyList(result);
}


void pyir_listExtend(PyObj* list, PyObj* items) { PyList::extend(list, &items, 1); }


void pyir_listAppend(PyObj* list, PyObj* item) { PyList::append(list, &item, 1); }


PyObj* pyir_buildSet(PyObj** parts, const int64_t count) {
    PySetData result;
    for (int64_t i = 0; i < count; i++) {
        parts[i]->incref();
        result.insert(parts[i]);
    }
    return new PySet(result);
}


void pyir_setUpdate(PyObj* set, PyObj* items) { PySet::update(set, &items, 1); }


void pyir_setAdd(PyObj* set, PyObj* item) { PySet::add(set, &item, 1); }
