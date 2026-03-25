//
// Created by matthew on 3/5/26.
//

#ifndef PYCOMPILE_PYIR_TYPES_H
#define PYCOMPILE_PYIR_TYPES_H

#include <llvm/ADT/TypeSwitch.h>
#include <mlir/IR/DialectImplementation.h>
#include <mlir/IR/Types.h>

#include "pyir/pyir.h"

#define GET_TYPEDEF_CLASSES
#include "pyir_types.h.inc"
#undef GET_TYPEDEF_CLASSES

#endif // PYCOMPILE_PYIR_TYPES_H
