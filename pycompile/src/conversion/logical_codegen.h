//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_LOGICAL_CODEGEN_H
#define PYCOMPILE_LOGICAL_CODEGEN_H

#include "bytecode/bytecode.h"
#include "conversion/pyir_codegen.h"


/**
 * Converts a BINARY_OP bytecode instruction to pyir::BinaryOp.
 */
void binaryOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a COMPARE_OP bytecode instruction to pyir::CompareOp.
 */
void compareOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a CONTAINS_OP bytecode instruction to pyir::ContainsOp.
 */
void containsOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a TO_BOOL bytecode instruction to pyir::ToBool.
 */
void toBoolCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta);


/**
 * Converts a UNARY_NOT bytecode instruction to pyir::UnaryNot.
 */
void unaryNotCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta);


/**
 * Converts a UNARY_NEGATIVE bytecode instruction to pyir::UnaryNegative.
 */
void unaryNegativeCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                          ConversionMeta& meta);


/**
 * Converts a UNARY_INVERT bytecode instruction to pyir::UnaryInvert.
 */
void unaryInvertCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        ConversionMeta& meta);


/**
 * Converts a FORMAT_SIMPLE bytecode instruction to pyir::FormatSimple.
 */
void formatSimpleCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         ConversionMeta& meta);

#endif // PYCOMPILE_LOGICAL_CODEGEN_H
