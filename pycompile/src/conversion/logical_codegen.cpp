//
// Created by matthew on 3/23/26.
//

#include "logical_codegen.h"

#include "../pyir/pyir_ops.h"
#include "../utils.h"


void binaryOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string opStr = instr.argrepr;
    if (opStr.empty())
        throw PyCompileError("BINARY_OP must have a string argval", loc);
    const mlir::Value rhs = meta.stack.back();
    meta.stack.pop_back();
    const mlir::Value lhs = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::BinaryOp::create(builder, loc, pyType, opStr, lhs, rhs).getResult());
}


void compareOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string opStr = instr.argrepr;
    if (opStr.empty())
        throw PyCompileError("COMPARE_OP must have a string argval", loc);
    const mlir::Value rhs = meta.stack.back();
    meta.stack.pop_back();
    const mlir::Value lhs = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::CompareOp::create(builder, loc, pyType, opStr, lhs, rhs).getResult());
}


void containsOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string opStr = instr.argrepr;
    if (opStr.empty())
        throw PyCompileError("CONTAINS_OP must have a string argval", loc);
    const mlir::Value container = meta.stack.back();
    meta.stack.pop_back();
    const mlir::Value element = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::ContainsOp::create(builder, loc, pyType, opStr, container, element).getResult());
}


void toBoolCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Value toConvert = meta.stack.back();
    meta.stack.pop_back(); // Replace value
    meta.stack.push_back(pyir::ToBool::create(builder, loc, pyType, toConvert).getResult());
}


void unaryNotCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::UnaryNot::create(builder, loc, pyType, value).getResult());
}


void unaryNegativeCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                          ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::UnaryNegative::create(builder, loc, pyType, value).getResult());
}


void unaryInvertCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::UnaryInvert::create(builder, loc, pyType, value).getResult());
}


void formatSimpleCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Value val = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(pyir::FormatSimple::create(builder, loc, pyType, val).getResult());
}
