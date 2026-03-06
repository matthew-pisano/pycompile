//
// Created by matthew on 3/5/26.
//

#include "pyir/pyir_ops.h"

#define GET_OP_CLASSES
#include "pyir_ops.h.inc"
#undef GET_OP_CLASSES

#define GET_OP_CLASSES
#include "pyir_ops.cpp.inc"
#undef GET_OP_CLASSES
