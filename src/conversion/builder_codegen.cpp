//
// Created by matthew on 3/23/26.
//

#include "conversion/builder_codegen.h"
#include "pyir/pyir_ops.h"


void buildStringCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* count = std::get_if<int64_t>(&instr.argval);
    if (!count)
        throw std::runtime_error("BUILD_STRING must have an int argval");

    std::vector<mlir::Value> parts(*count);
    for (int64_t i = *count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(builder.create<pyir::BuildString>(loc, pyType, parts).getResult());
}


void buildListCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* count = std::get_if<int64_t>(&instr.argval);
    if (!count)
        throw std::runtime_error("BUILD_LIST must have an int argval");

    std::vector<mlir::Value> parts(*count);
    for (int64_t i = *count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(builder.create<pyir::BuildList>(loc, pyType, parts).getResult());
}


void listExtendCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* count = std::get_if<int64_t>(&instr.argval);
    if (!count)
        throw std::runtime_error("LIST_EXTEND must have an int argval");

    std::vector<mlir::Value> parts(*count);
    for (int64_t i = *count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(builder.create<pyir::ListExtend>(loc, pyType, parts).getResult());
}
