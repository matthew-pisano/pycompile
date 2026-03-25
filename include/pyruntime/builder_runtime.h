//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_RUNTIME_H
#define PYCOMPILE_BUILDER_RUNTIME_H

#include "pyir/pyir_value.h"

extern "C" {

Value* pyir_buildString(Value** parts, int64_t count);

}

#endif //PYCOMPILE_BUILDER_RUNTIME_H
