//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_FUNCTION_CODEGEN_H
#define PYCOMPILE_FUNCTION_CODEGEN_H
#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "conversion/pyir_codegen.h"
#include "bytecode/bytecode.h"


/**
 * Converts a MAKE_FUNCTION bytecode instruction to pyir::MakeFunction.
 */
void makeFunctionCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         ConversionMeta& meta);


/**
 * Converts a CALL bytecode instruction to pyir::Call.
 */
void callFuncCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a RETURN_VALUE bytecode instruction to an operation dependent on what is being returned and where from.
 *
 * For functions, pyir::PopScope and pyir::ReturnValue are used and for modules, pyir::DestroyModule and
 * mlir::func::ReturnOp.
 */
void returnValueCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, ConversionMeta& meta);

#endif // PYCOMPILE_FUNCTION_CODEGEN_H
