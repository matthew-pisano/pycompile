//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_CONTROL_FLOW_CODEGEN_H
#define PYCOMPILE_CONTROL_FLOW_CODEGEN_H

#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "bytecode/bytecode.h"
#include "pyir_codegen.h"


/**
 * Converts a JUMP_FORWARD bytecode instruction to mlir::cf::BranchOp.
 */
void jumpForwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                        const ConversionMeta& meta);


/**
 * Converts a JUMP_BACKWARD bytecode instruction to mlir::cf::BranchOp.
 */
void jumpBackwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                         const ConversionMeta& meta);


/**
 * Converts a POP_JUMP_IF_TRUE bytecode instruction to mlir::cf::BranchOp.
 */
void popJumpIfTrueCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                          const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a POP_JUMP_IF_FALSE bytecode instruction to mlir::cf::BranchOp.
 */
void popJumpIfFalseCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                           const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a GET_ITER bytecode instruction to pyir::GetIter.
 */
void getIterCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta);


/**
 * Converts a FOR_ITER bytecode instruction to pyir::ForIter.
 */
void forIterCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, mlir::func::FuncOp& fn,
                    const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a POP_ITER bytecode instruction to pyir::PopIter.
 */
void popIterCodegen(mlir::OpBuilder& builder, const mlir::Location& loc);

#endif // PYCOMPILE_CONTROL_FLOW_CODEGEN_H
