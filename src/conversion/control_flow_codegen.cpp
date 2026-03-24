//
// Created by matthew on 3/23/26.
//

#include "conversion/control_flow_codegen.h"

#include <variant>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>

#include "utils.h"
#include "pyir/pyir_ops.h"


void jumpForwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                        const ConversionMeta& meta) {
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("JUMP_FORWARD must have an int argval", loc);
    mlir::Block* dest = meta.offsetToBlock.at(*target);
    builder.create<mlir::cf::BranchOp>(loc, dest);
}


inline void popJumpIfCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                             const int64_t* target, const bool truthy, ConversionMeta& meta) {
    mlir::Value cond = meta.stack.back();
    meta.stack.pop_back();

    // unbox !pyir.object -> i1
    mlir::Value i1cond = builder.create<pyir::IsTruthy>(loc, builder.getI1Type(), cond).getResult();
    mlir::Block* ifBlock = meta.offsetToBlock.at(*target);
    mlir::Block* elseBlock = fn.addBlock();

    // Pass remaining stack values as block arguments to the if block so they are available after the jump
    llvm::SmallVector<mlir::Value> ifArgs(meta.stack.begin(), meta.stack.end());
    llvm::SmallVector<mlir::Type> ifArgTypes;
    for (mlir::Value v : ifArgs)
        ifArgTypes.push_back(v.getType());

    if (ifBlock->getNumArguments() == 0)
        for (const mlir::Type t : ifArgTypes)
            ifBlock->addArgument(t, loc);

    if (truthy)
        builder.create<mlir::cf::CondBranchOp>(loc, i1cond, ifBlock, mlir::ValueRange{ifArgs},
                                               elseBlock, mlir::ValueRange{});
    else
        builder.create<mlir::cf::CondBranchOp>(loc, i1cond, elseBlock, mlir::ValueRange{}, ifBlock,
                                               mlir::ValueRange{ifArgs});
    builder.setInsertionPointToStart(elseBlock);
}


void popJumpIfTrueCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                          const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("POP_JUMP_IF_TRUE must have an int argval", loc);
    popJumpIfCodegen(builder, loc, fn, target, true, meta);
}


void popJumpIfFalseCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, mlir::func::FuncOp& fn,
                           const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("POP_JUMP_IF_FALSE must have an int argval", loc);
    popJumpIfCodegen(builder, loc, fn, target, false, meta);
}
