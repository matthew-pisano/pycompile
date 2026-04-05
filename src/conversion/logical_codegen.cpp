//
// Created by matthew on 3/23/26.
//

#include "conversion/logical_codegen.h"

#include "pyir/pyir_ops.h"
#include "utils.h"


void binaryOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string opStr = instr.argrepr;
    if (opStr.empty())
        throw PyCompileError("BINARY_OP must have a string argval", loc);
    mlir::Value rhs = meta.stack.back();
    meta.stack.pop_back();
    mlir::Value lhs = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::BinaryOp>(loc, pyType, opStr, lhs, rhs).getResult());
}


void compareOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string opStr = instr.argrepr;
    if (opStr.empty())
        throw PyCompileError("COMPARE_OP must have a string argval", loc);
    mlir::Value rhs = meta.stack.back();
    meta.stack.pop_back();
    mlir::Value lhs = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::CompareOp>(loc, pyType, opStr, lhs, rhs).getResult());
}


void containsOpCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string opStr = instr.argrepr;
    if (opStr.empty())
        throw PyCompileError("CONTAINS_OP must have a string argval", loc);
    mlir::Value container = meta.stack.back();
    meta.stack.pop_back();
    mlir::Value element = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::ContainsOp>(loc, pyType, opStr, container, element).getResult());
}


void toBoolCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    mlir::Value toConvert = meta.stack.back();
    meta.stack.pop_back(); // Replace value
    meta.stack.push_back(builder.create<pyir::ToBool>(loc, pyType, toConvert).getResult());
}


void unaryNotCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::UnaryNot>(loc, pyType, value).getResult());
}


void unaryNegativeCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                          ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::UnaryNegative>(loc, pyType, value).getResult());
}


void unaryInvertCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::UnaryInvert>(loc, pyType, value).getResult());
}


void formatSimpleCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    mlir::Value val = meta.stack.back();
    meta.stack.pop_back();
    meta.stack.push_back(builder.create<pyir::FormatSimple>(loc, pyType, val).getResult());
}
