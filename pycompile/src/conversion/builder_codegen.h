//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_BUILDER_CODEGEN_H
#define PYCOMPILE_BUILDER_CODEGEN_H

#include "pyir_codegen.h"
#include "../bytecode/bytecode.h"


/**
 * Converts a BUILD_STRING bytecode instruction to pyir::BuildString.
 */
void buildStringCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a BUILD_LIST bytecode instruction to pyir::BuildList.
 */
void buildListCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a LIST_EXTEND bytecode instruction to pyir::ListExtend.
 */
void listExtendCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       ConversionMeta& meta);


/**
 * Converts a LIST_APPEND bytecode instruction to pyir::ListAppend.
 */
void listAppendCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       ConversionMeta& meta);


/**
 * Converts a BUILD_SET bytecode instruction to pyir::BuildSet.
 */
void buildSetCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a SET_UPDATE bytecode instruction to pyir::SetUpdate.
 */
void setUpdateCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta);


/**
 * Converts a SET_ADD bytecode instruction to pyir::SetAdd.
 */
void setAddCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                   ConversionMeta& meta);


/**
 * Converts a BUILD_MAP bytecode instruction to pyir::BuildMap.
 */
void buildMapCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a MAP_ADD bytecode instruction to pyir::MapAdd.
 */
void mapAddCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                   ConversionMeta& meta);

#endif // PYCOMPILE_BUILDER_CODEGEN_H
