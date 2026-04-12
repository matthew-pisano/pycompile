//
// Created by matthew on 3/23/26.
//

#include "conversion/control_flow_codegen.h"

#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <variant>

#include "pyir/pyir_ops.h"
#include "utils.h"


void jumpForwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                        const ConversionMeta& meta) {
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("JUMP_FORWARD must have an int argval", loc);
    mlir::Block* dest = meta.offsetToBlock.at(*target);
    builder.create<mlir::cf::BranchOp>(loc, dest);
}


void jumpBackwardCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                         const ConversionMeta& meta) {
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("JUMP_BACKWARD must have an int argval", loc);
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
        builder.create<mlir::cf::CondBranchOp>(loc, i1cond, ifBlock, mlir::ValueRange{ifArgs}, elseBlock,
                                               mlir::ValueRange{});
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


void getIterCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    mlir::Value container = meta.stack.back();
    meta.stack.pop_back(); // Replace value
    meta.stack.push_back(builder.create<pyir::GetIter>(loc, pyType, container).getResult());
}


void forIterCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, mlir::func::FuncOp& fn,
                    const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("FOR_ITER must have an int argval", loc);

    // Get iterator for instruction
    mlir::Value iterator = meta.stack.back();

    mlir::Block* exitBlock = meta.offsetToBlock.at(*target);
    mlir::Block* bodyBlock = fn.addBlock();

    // Body block receives the next value as a block argument
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    bodyBlock->addArgument(pyType, loc);

    // Emit for_iter op: advances iterator, branches to body with next value or to exit
    builder.create<pyir::ForIter>(loc, iterator, bodyBlock, exitBlock);

    // Continue emission in body block, with next value on top of stack
    builder.setInsertionPointToStart(bodyBlock);
    // Push the next value (block argument) onto the stack
    meta.stack.push_back(bodyBlock->getArgument(0));
}
