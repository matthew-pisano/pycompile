//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_RUNTIME_H
#define PYCOMPILE_BUILDER_RUNTIME_H
#include <cstdint>


extern "C" {

struct PyObj;

PyObj* pyir_buildString(PyObj** parts, int64_t count);

void pyir_listExtend(PyObj* list, const PyObj* items);

void pyir_listAppend(PyObj* list, PyObj* item);
}

#endif // PYCOMPILE_BUILDER_RUNTIME_H
