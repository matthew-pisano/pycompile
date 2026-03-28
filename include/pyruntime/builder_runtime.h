//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_RUNTIME_H
#define PYCOMPILE_BUILDER_RUNTIME_H

#include "pyir_value.h"

extern "C" {

Value* pyir_buildString(Value** parts, int64_t count);

Value* pyir_buildList(Value** parts, int64_t count);

void pyir_listExtend(Value* list, const Value* items);

void pyir_listAppend(Value* list, Value* item);
}

#endif // PYCOMPILE_BUILDER_RUNTIME_H
