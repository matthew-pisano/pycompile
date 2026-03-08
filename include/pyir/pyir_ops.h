//
// Created by matthew on 3/5/26.
//

#ifndef PYCOMPILE_PYIR_OPS_H
#define PYCOMPILE_PYIR_OPS_H

#include <mlir/Bytecode/BytecodeOpInterface.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinAttributes.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/OpDefinition.h>
#include <mlir/IR/OpImplementation.h>
#include <mlir/Interfaces/SideEffectInterfaces.h>

#include "pyir/pyir_types.h"

#define GET_OP_CLASSES
#include "pyir_ops.h.inc"
#undef GET_OP_CLASSES

#endif //PYCOMPILE_PYIR_OPS_H
