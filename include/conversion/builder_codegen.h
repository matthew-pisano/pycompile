//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_BUILDER_CODEGEN_H
#define PYCOMPILE_BUILDER_CODEGEN_H

#include <mlir/IR/Builders.h>

#include "codegen_utils.h"
#include  "bytecode/bytecode.h"


void buildStringCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        const ByteCodeInstruction& instr, ConversionMeta& meta);

#endif //PYCOMPILE_BUILDER_CODEGEN_H
