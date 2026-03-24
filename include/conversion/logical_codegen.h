//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_LOGICAL_CODEGEN_H
#define PYCOMPILE_LOGICAL_CODEGEN_H

#include <mlir/IR/Builders.h>

#include "codegen_utils.h"
#include "bytecode/bytecode.h"


void binaryOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


void compareOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta);


void toBoolCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta);


void unaryNotCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     ConversionMeta& meta);


void unaryNegativeCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                          ConversionMeta& meta);


void unaryInvertCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        ConversionMeta& meta);


void formatSimpleCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         ConversionMeta& meta);

#endif //PYCOMPILE_LOGICAL_CODEGEN_H
