//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_FUNCTION_CODEGEN_H
#define PYCOMPILE_FUNCTION_CODEGEN_H
#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "bytecode/bytecode.h"
#include "codegen_utils.h"


void makeFunctionCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         ConversionMeta& meta);


void callFuncCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


void returnValueCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, ConversionMeta& meta);

#endif // PYCOMPILE_FUNCTION_CODEGEN_H
