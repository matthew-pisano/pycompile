//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_MEMORY_CODEGEN_H
#define PYCOMPILE_MEMORY_CODEGEN_H

#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "bytecode/bytecode.h"
#include "codegen_utils.h"


void loadSmallIntCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         const ByteCodeInstruction& instr, ConversionMeta& meta);


void loadGlobalCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta);


void loadFastBorrowCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                           const ByteCodeInstruction& instr, ConversionMeta& meta);


void loadNameCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


void storeNameCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta);


void loadFastCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


void storeFastCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta);


void loadDerefCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, const CodeInfo& codeInfo, ConversionMeta& meta);


void storeDerefCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       const CodeInfo& codeInfo, ConversionMeta& meta);


void loadConstCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const mlir::func::FuncOp& fn, const ByteCodeInstruction& instr, ConversionMeta& meta);


void loadAttrCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);

#endif // PYCOMPILE_MEMORY_CODEGEN_H
