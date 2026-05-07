//
// Created by matthew on 3/23/26.
//

#ifndef PYCOMPILE_MEMORY_CODEGEN_H
#define PYCOMPILE_MEMORY_CODEGEN_H

#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "bytecode/bytecode.h"
#include "pyir_codegen.h"


/**
 * Converts a LOAD_SMALL_INT bytecode instruction to pyir::LoadConst.
 */
void loadSmallIntCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                         const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a LOAD_GLOBAL bytecode instruction to pyir::LoadName and pyir::PushNull.
 */
void loadGlobalCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                       const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a LOAD_FAST_BORROW bytecode instruction to pyir::LoadFast.
 */
void loadFastBorrowCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                           const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a LOAD_NAME bytecode instruction to pyir::LoadName.
 */
void loadNameCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a STORE_NAME bytecode instruction to pyir::StoreName.
 */
void storeNameCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta);


/**
 * Converts a LOAD_FAST bytecode instruction to pyir::LoadFast.
 */
void loadFastCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a STORE_FAST bytecode instruction to pyir::StoreFast.
 */
void storeFastCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                      ConversionMeta& meta);


/**
 * Converts a LOAD_DEREF bytecode instruction to pyir::LoadDeref.
 */
void loadDerefCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const ByteCodeInstruction& instr, const CodeInfo& codeInfo, ConversionMeta& meta);


/**
 * Converts a STORE_DEREF bytecode instruction to pyir::StoreDeref.
 */
void storeDerefCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, const ByteCodeInstruction& instr,
                       const CodeInfo& codeInfo, ConversionMeta& meta);


/**
 * Converts a LOAD_CONST bytecode instruction to pyir::LoadConst or creates a nested module if a function.
 */
void loadConstCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                      const mlir::func::FuncOp& fn, const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a LOAD_ATTR bytecode instruction to pyir::LoadAttr and pyir::PushNull.
 */
void loadAttrCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                     const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a STORE_SUBSCR bytecode instruction to pyir::StoreSubscr.
 */
void storeSubscrCodegen(mlir::OpBuilder& builder, const mlir::Location& loc, ConversionMeta& meta);


/**
 * Converts a STORE_FAST_LOAD_FAST bytecode instruction to pyir::StoreFast followed by pyir::LoadFast.
 */
void storeFastLoadFastCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                              const ByteCodeInstruction& instr, ConversionMeta& meta);


/**
 * Converts a LOAD_FAST_AND_CLEAR bytecode instruction to pyir::LoadFast and pyir::StoreFast with null.
 */
void loadFastAndClearCodegen(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const mlir::Location& loc,
                             const ByteCodeInstruction& instr, ConversionMeta& meta);

#endif // PYCOMPILE_MEMORY_CODEGEN_H
