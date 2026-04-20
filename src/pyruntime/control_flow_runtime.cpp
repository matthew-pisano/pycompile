//
// Created by matthew on 4/12/26.
//

#include "pyruntime/control_flow_runtime.h"

#include <stdexcept>
#include <vector>

#include "pyruntime/builtin_runtime.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/runtime_errors.h"

static std::vector<PyObj*> iterStack = {};

PyObj* pyir_getIter(PyObj* container) {
    iterStack.push_back(pyir_builtinIter(&container, 1));
    return iterStack.back();
}

PyObj* pyir_forIter(PyObj* iterator) {
    try {
        return pyir_builtinNext(&iterator, 1);
    } catch (const PyStopIteration&) {
        return nullptr;
    }
}

PyObj* pyir_popIter() {
    if (!iterStack.back()->decref())
        throw std::runtime_error("Invalid pop on referenced iter");
    iterStack.pop_back();
    return PyNone::None;
}
