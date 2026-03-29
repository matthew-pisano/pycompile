//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_RUNTIME_H
#define PYCOMPILE_BUILDER_RUNTIME_H

#include "runtime_value.h"

extern "C" {

PyValue* pyir_buildString(PyValue** parts, int64_t count);

PyValue* pyir_buildList(PyValue** parts, int64_t count);

void pyir_listExtend(PyValue* list, const PyValue* items);

void pyir_listAppend(PyValue* list, PyValue* item);
}

#endif // PYCOMPILE_BUILDER_RUNTIME_H
