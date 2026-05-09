//
// Created by matthew on 3/23/26.
//

#include "conversion/builder_codegen.h"
#include "pyir/pyir_ops.h"


void buildStringCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                        const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* count = std::get_if<int64_t>(&instr.argval);
    if (!count)
        throw std::runtime_error("BUILD_STRING must have an int argval");

    std::vector<mlir::Value> parts(*count);
    for (int64_t i = *count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(pyir::BuildString::create(builder, loc, pyType, parts).getResult());
}


void buildListCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* count = std::get_if<int64_t>(&instr.argval);
    if (!count)
        throw std::runtime_error("BUILD_LIST must have an int argval");

    std::vector<mlir::Value> parts(*count);
    for (int64_t i = *count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(pyir::BuildList::create(builder, loc, pyType, parts).getResult());
}


void listExtendCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       ConversionMeta& meta) {
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw std::runtime_error("LIST_EXTEND must have an int argval");

    const mlir::Value items = meta.stack.back();
    meta.stack.pop_back();

    const mlir::Value list = meta.stack.at(meta.stack.size() - *idx);
    pyir::ListExtend::create(builder, loc, list, items);
}


void listAppendCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       ConversionMeta& meta) {
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw std::runtime_error("LIST_APPEND must have an int argval");

    const mlir::Value item = meta.stack.back();
    meta.stack.pop_back();

    const mlir::Value list = meta.stack.at(meta.stack.size() - *idx);
    pyir::ListAppend::create(builder, loc, list, item);
}


void buildSetCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* count = std::get_if<int64_t>(&instr.argval);
    if (!count)
        throw std::runtime_error("BUILD_SET must have an int argval");

    std::vector<mlir::Value> parts(*count);
    for (int64_t i = *count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(pyir::BuildSet::create(builder, loc, pyType, parts).getResult());
}


void setUpdateCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta) {
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw std::runtime_error("SET_UPDATE must have an int argval");

    const mlir::Value items = meta.stack.back();
    meta.stack.pop_back();

    const mlir::Value set = meta.stack.at(meta.stack.size() - *idx);
    pyir::SetUpdate::create(builder, loc, set, items);
}


void setAddCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                   ConversionMeta& meta) {
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw std::runtime_error("SET_ADD must have an int argval");

    const mlir::Value item = meta.stack.back();
    meta.stack.pop_back();

    const mlir::Value set = meta.stack.at(meta.stack.size() - *idx);
    pyir::SetAdd::create(builder, loc, set, item);
}


void buildMapCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    const pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* countPtr = std::get_if<int64_t>(&instr.argval);
    if (!countPtr)
        throw std::runtime_error("BUILD_MAP must have an int argval");

    int64_t count = *countPtr;
    count *= 2; // Double count since Python counts in pairs, not individual objects
    std::vector<mlir::Value> parts(count);
    for (int64_t i = count - 1; i >= 0; i--) {
        parts[i] = meta.stack.back();
        meta.stack.pop_back();
    }
    meta.stack.push_back(pyir::BuildMap::create(builder, loc, pyType, parts).getResult());
}


void mapAddCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                   ConversionMeta& meta) {
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw std::runtime_error("MAP_ADD must have an int argval");

    const mlir::Value value = meta.stack.back();
    meta.stack.pop_back();
    const mlir::Value key = meta.stack.back();
    meta.stack.pop_back();

    const mlir::Value dict = meta.stack.at(meta.stack.size() - *idx);
    pyir::MapAdd::create(builder, loc, dict, key, value);
}
