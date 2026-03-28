//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_BUILDER_CODEGEN_H
#define PYCOMPILE_BUILDER_CODEGEN_H

#include <mlir/IR/Builders.h>

#include "bytecode/bytecode.h"
#include "codegen_utils.h"


void buildStringCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        const ByteCodeInstruction& instr, ConversionMeta& meta);


void buildListCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta);


void listExtendCodegen(mlir::OpBuilder& builder, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta);


void listAppendCodegen(mlir::OpBuilder& builder, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta);

#endif // PYCOMPILE_BUILDER_CODEGEN_H
