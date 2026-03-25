//
// Created by matthew on 3/23/26.
//

#include "conversion/memory_codegen.h"

#include <filesystem>
#include <string>
#include <variant>

#include "conversion/pyir_codegen.h"
#include "pyir/pyir_attrs.h"
#include "pyir/pyir_ops.h"
#include "utils.h"


/// Global counter to build unique function names
static size_t fnCounter = 0;


/**
 * Resolves free or cell variable names based on the given CodeInfo
 * @param info The CodeInfo of the program
 * @param idx The index of the desired name
 * @return The name of the free or cell variable
 */
std::string resolveDeref(const CodeInfo& info, const int64_t idx) {
    const int cellS_start = static_cast<int>(info.varnames.size());
    const int freeStart = cellS_start + static_cast<int>(info.cellvars.size());
    if (idx < cellS_start)
        return info.varnames[idx];
    if (idx < freeStart)
        return info.cellvars[idx - cellS_start];
    return info.freevars[idx - freeStart];
}


void loadSmallIntCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* target = std::get_if<int64_t>(&instr.argval);
    if (!target)
        throw PyCompileError("LOAD_SMALL_INT must have an int argval", loc);
    mlir::Attribute attr = builder.getI64IntegerAttr(*target);
    meta.stack.push_back(builder.create<pyir::LoadConst>(loc, pyType, attr).getResult());
}


void loadGlobalCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string* name = std::get_if<std::string>(&instr.argval);
    if (!name)
        throw PyCompileError("LOAD_GLOBAL must have a string argval", loc);
    // Push the value
    meta.stack.push_back(builder.create<pyir::LoadName>(loc, pyType, *name).getResult());
    // LOAD_GLOBAL also pushes a null sentinel
    meta.stack.push_back(builder.create<pyir::PushNull>(loc, pyType).getResult());
}


void loadFastBorrowCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                           const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string* name = std::get_if<std::string>(&instr.argval);
    if (!name)
        throw PyCompileError("LOAD_FAST_BORROW must have a string argval", loc);
    meta.stack.push_back(builder.create<pyir::LoadFast>(loc, pyType, *name).getResult());
}


void loadNameCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string* name = std::get_if<std::string>(&instr.argval);
    if (!name)
        throw PyCompileError("LOAD_NAME must have a string argval", loc);
    meta.stack.push_back(builder.create<pyir::LoadName>(loc, pyType, *name).getResult());
}


void storeNameCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta) {
    const std::string* name = std::get_if<std::string>(&instr.argval);
    if (!name)
        throw PyCompileError("STORE_NAME must have a string argval", loc);
    mlir::Value val = meta.stack.back();
    meta.stack.pop_back();
    builder.create<pyir::StoreName>(loc, *name, val);
}


void loadFastCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const std::string* name = std::get_if<std::string>(&instr.argval);
    if (!name)
        throw PyCompileError("LOAD_FAST must have a string argvall", loc);
    meta.stack.push_back(builder.create<pyir::LoadFast>(loc, pyType, *name).getResult());
}


void storeFastCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta) {
    const std::string* name = std::get_if<std::string>(&instr.argval);
    if (!name)
        throw PyCompileError("STORE_FAST must have a string argval", loc);
    mlir::Value val = meta.stack.back();
    meta.stack.pop_back();
    builder.create<pyir::StoreFast>(loc, *name, val);
}


void loadDerefCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, const CodeInfo& codeInfo, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw PyCompileError("LOAD_DEREF must have an int argval", loc);
    const std::string name = resolveDeref(codeInfo, *idx);
    meta.stack.push_back(builder.create<pyir::LoadDeref>(loc, pyType, name).getResult());
}


void storeDerefCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       const CodeInfo& codeInfo, ConversionMeta& meta) {
    const int64_t* idx = std::get_if<int64_t>(&instr.argval);
    if (!idx)
        throw PyCompileError("STORE_DEREF must have an int argval", loc);
    const std::string name = resolveDeref(codeInfo, *idx);
    mlir::Value val = meta.stack.back();
    meta.stack.pop_back();
    builder.create<pyir::StoreDeref>(loc, name, val);
}


void loadConstCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const mlir::func::FuncOp& fn, const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    if (std::holds_alternative<ArgvalNone>(instr.argval)) {
        // None is a valid return value inside functions
        if (meta.isFunction) {
            mlir::Attribute attr = pyir::NoneAttr::get(&ctx);
            meta.stack.push_back(builder.create<pyir::LoadConst>(loc, pyType, attr).getResult());
        }
        return;
    }

    // Nested code object: emit a child FuncOp and push its name for MAKE_FUNCTION
    if (const auto* nestedPtr = std::get_if<std::shared_ptr<ByteCodeModule>>(&instr.argval)) {
        const ByteCodeModule& nested = **nestedPtr;

        // Mangle a unique name: __pyfn_<funcname>_<counter>
        const std::string fnName = "__pyfn_" + nested.info.codeName + "_" + std::to_string(fnCounter++);

        // Save insertion point, emit the nested FuncOp at module level, then restore
        mlir::OpBuilder::InsertionGuard guard(builder);
        mlir::ModuleOp parentModule = fn->getParentOfType<mlir::ModuleOp>();
        builder.setInsertionPointToEnd(parentModule.getBody());
        buildMLIRModule(builder, ctx, nested, fnName);

        const mlir::Value sentinel = builder.create<pyir::PushNull>(loc, pyType).getResult();
        meta.pendingFunctions[static_cast<mlir::Value*>(sentinel.getAsOpaquePointer())] = fnName;
        meta.stack.push_back(sentinel);
        return;
    }

    mlir::Attribute attr;
    if (const std::string* s = std::get_if<std::string>(&instr.argval))
        attr = builder.getStringAttr(*s);
    else if (const int64_t* i = std::get_if<int64_t>(&instr.argval))
        attr = builder.getI64IntegerAttr(*i);
    else if (const double_t* f = std::get_if<double_t>(&instr.argval))
        attr = builder.getF64FloatAttr(*f);
    else if (const bool* b = std::get_if<bool>(&instr.argval))
        attr = builder.getBoolAttr(*b);

    meta.stack.push_back(builder.create<pyir::LoadConst>(loc, pyType, attr).getResult());
}
