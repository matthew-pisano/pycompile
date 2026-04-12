//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_CONTROL_FLOW_CODEGEN_H
#define PYCOMPILE_CONTROL_FLOW_CODEGEN_H

#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "bytecode/bytecode.h"
#include "codegen_utils.h"


void jumpForwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                        const ConversionMeta& meta);


void jumpBackwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                         const ConversionMeta& meta);


void popJumpIfTrueCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                          const ByteCodeInstruction& instr, ConversionMeta& meta);


void popJumpIfFalseCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                           const ByteCodeInstruction& instr, ConversionMeta& meta);


void getIterCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta);


void forIterCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, mlir::func::FuncOp& fn,
                    const ByteCodeInstruction& instr, ConversionMeta& meta);

#endif // PYCOMPILE_CONTROL_FLOW_CODEGEN_H
