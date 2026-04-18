//
// Created by matthew on 4/12/26.
//

#include "pyruntime/control_flow_runtime.h"

#include "pyruntime/builtin_runtime.h"
#include "pyruntime/runtime_errors.h"


PyObj* pyir_getIter(PyObj* container) { return pyir_builtinIter(&container, 1); }

PyObj* pyir_forIter(PyObj* iterator) {
    try {
        return pyir_builtinNext(&iterator, 1);
    } catch (const PyStopIteration&) {
        return nullptr;
    }
}
