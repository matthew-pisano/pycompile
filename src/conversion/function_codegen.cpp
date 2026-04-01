//
// Created by matthew on 3/23/26.
//

#include "conversion/function_codegen.h"

#include <mlir/IR/Value.h>
#include <string>
#include <variant>

#include "pyir/pyir_ops.h"
#include "utils.h"


void makeFunctionCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         const std::string& displayName, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Value sentinel = meta.stack.back();
    meta.stack.pop_back();

    const std::string fnName = meta.pendingFunctions.at(static_cast<mlir::Value*>(sentinel.getAsOpaquePointer()));
    meta.pendingFunctions.erase(static_cast<mlir::Value*>(sentinel.getAsOpaquePointer()));
    // Erase the sentinel PushNull op since it was just a placeholder
    sentinel.getDefiningOp()->erase();

    meta.stack.push_back(builder.create<pyir::MakeFunction>(loc, pyType, displayName, fnName).getResult());
}


void callFuncCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const int64_t* argc = std::get_if<int64_t>(&instr.argval);
    if (!argc)
        throw PyCompileError("CALL must have an int argval", loc);
    // Pop arguments in reverse order
    std::vector<mlir::Value> args(*argc);
    for (int64_t i = *argc - 1; i >= 0; i--) {
        args[i] = meta.stack.back();
        meta.stack.pop_back();
    }

    meta.stack.pop_back(); // null sentinel
    mlir::Value callee = meta.stack.back();
    meta.stack.pop_back(); // actual callee

    meta.stack.push_back(builder.create<pyir::Call>(loc, pyType, callee, args).getResult());
}


void returnValueCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, ConversionMeta& meta) {
    mlir::Value retVal;
    if (!meta.stack.empty()) {
        retVal = meta.stack.back();
        meta.stack.pop_back();
    }

    // Pop the local scope before returning from a function (not needed at module level)
    if (meta.isFunction)
        builder.create<pyir::PopScope>(loc);

    if (retVal)
        builder.create<pyir::ReturnValue>(loc, retVal);
    else
        builder.create<mlir::func::ReturnOp>(loc);
}
