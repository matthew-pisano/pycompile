//
// Created by matthew on 3/5/26.
//

#include "pyir/pyir_ops.h"

#include "pyir_ops.h.inc"

#define GET_OP_CLASSES
#include "pyir_ops.cpp.inc"
#undef GET_OP_CLASSES


void pyir::Call::getEffects(
        llvm::SmallVectorImpl<mlir::SideEffects::EffectInstance<mlir::MemoryEffects::Effect> >& effects) {
    // calls into the runtime may read/write memory and produce output
    effects.emplace_back(mlir::MemoryEffects::Write::get());
    effects.emplace_back(mlir::MemoryEffects::Read::get());
}
