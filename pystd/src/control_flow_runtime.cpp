//
// Created by matthew on 4/12/26.
//

#include "control_flow_runtime.h"

#include <stdexcept>
#include <vector>

#include "builtin_runtime.h"
#include "pytypes/primitives/py_none.h"
#include "pytypes/py_object.h"
#include "runtime_errors.h"

static std::vector<PyObj*> iterStack = {};

PyObj* pyir_getIter(PyObj* container) {
    iterStack.push_back(pyir_builtinIter(&container, 1));
    return iterStack.back();
}

PyObj* pyir_forIter(PyObj* iterator) {
    iterator->incref();
    try {
        // Get next value to be returned to MLIR where its existence is used as a branch condition
        return pyir_builtinNext(&iterator, 1);
    } catch (const PyStopIteration&) {
        (void) iterator->decref();
        return nullptr;
    }
}

PyObj* pyir_popIter() {
    if (!iterStack.back()->decref())
        throw std::runtime_error("Invalid pop on referenced iter");
    iterStack.pop_back();
    return PyNone::None;
}
