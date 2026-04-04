//
// Created by matthew on 3/6/26.
//

#include "conversion/pyir_codegen.h"

#include <filesystem>
#include <iostream>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/IR/AsmState.h>
#include <mlir/IR/Verifier.h>

#include "conversion/builder_codegen.h"
#include "conversion/codegen_utils.h"
#include "conversion/control_flow_codegen.h"
#include "conversion/function_codegen.h"
#include "conversion/logical_codegen.h"
#include "conversion/memory_codegen.h"
#include "pyir/pyir_attrs.h"
#include "pyir/pyir_ops.h"
#include "utils.h"

/**
 * Mangles a Python module name to work as a valid MLIR symbol
 * @param filename The path to the Python module
 * @return
 */
std::string mangleModuleName(const std::string& filename) {
    std::string result = "__pymodule_";
    for (const char c : filename) {
        if (std::isalnum(c))
            result += c;
        else
            result += '_';
    }
    return result;
}

/**
 * Gets the location of the given instruction in terms of MLIR
 * @param ctx The MLIR context
 * @param instr The instruction to locate
 * @param filename The filename for the instruction
 * @return An MLIR location
 */
mlir::Location getInstructionLocation(mlir::MLIRContext& ctx, const ByteCodeInstruction& instr,
                                      const std::string& filename) {
    if (instr.lineno.has_value())
        return mlir::FileLineColLoc::get(&ctx, llvm::StringRef(filename), *instr.lineno, 0);
    return mlir::UnknownLoc::get(&ctx);
}


/**
 * Builds a single MLIR instruction from the given bytecode instruction, adding it through the builder.
 */
void buildMLIRInstruction(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module,
                          mlir::func::FuncOp& fn, const ByteCodeInstruction& instr, ConversionMeta& meta) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);
    const mlir::Location loc = getInstructionLocation(ctx, instr, module.filename);

    switch (instr.opcode) {
        case PythonOpcode::RESUME:
        case PythonOpcode::NOT_TAKEN:
            // Bookkeeping instructions, no MLIR equivalent needed.
            return;
        case PythonOpcode::PUSH_NULL:
            // Push a null sentinel onto the stack for the call convention.
            return meta.stack.push_back(builder.create<pyir::PushNull>(loc, pyType).getResult());
        case PythonOpcode::POP_TOP:
            // Discard top of stack, if the value is unused MLIR's DCE will clean up the producing op if it's Pure.
            return meta.stack.pop_back();
        case PythonOpcode::COPY: {
            const int64_t* copyIdx = std::get_if<int64_t>(&instr.argval);
            return meta.stack.push_back(meta.stack.at(meta.stack.size() - *copyIdx));
        }
        case PythonOpcode::MAKE_FUNCTION:
            return makeFunctionCodegen(builder, ctx, loc, meta);
        case PythonOpcode::LOAD_GLOBAL:
            return loadGlobalCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::LOAD_FAST_BORROW:
            return loadFastBorrowCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::LOAD_CONST:
            return loadConstCodegen(builder, ctx, loc, fn, instr, meta);
        case PythonOpcode::LOAD_ATTR:
            return loadAttrCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::LOAD_NAME:
            return loadNameCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::STORE_NAME:
            return storeNameCodegen(builder, loc, instr, meta);
        case PythonOpcode::LOAD_FAST:
            return loadFastCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::STORE_FAST:
            return storeFastCodegen(builder, loc, instr, meta);
        case PythonOpcode::LOAD_DEREF:
            return loadDerefCodegen(builder, ctx, loc, instr, module.info, meta);
        case PythonOpcode::STORE_DEREF:
            return storeDerefCodegen(builder, loc, instr, module.info, meta);
        case PythonOpcode::BINARY_OP:
            return binaryOpCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::COMPARE_OP:
            return compareOpCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::CONTAINS_OP:
            return containsOpCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::CALL:
            return callFuncCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::RETURN_VALUE:
            return returnValueCodegen(builder, loc, meta);
        case PythonOpcode::JUMP_FORWARD:
            return jumpForwardCodegen(builder, loc, instr, meta);
        case PythonOpcode::POP_JUMP_IF_TRUE:
            return popJumpIfTrueCodegen(builder, loc, fn, instr, meta);
        case PythonOpcode::POP_JUMP_IF_FALSE:
            return popJumpIfFalseCodegen(builder, loc, fn, instr, meta);
        case PythonOpcode::LOAD_SMALL_INT:
            return loadSmallIntCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::TO_BOOL:
            return toBoolCodegen(builder, ctx, loc, meta);
        case PythonOpcode::UNARY_NOT:
            return unaryNotCodegen(builder, ctx, loc, meta);
        case PythonOpcode::UNARY_NEGATIVE:
            return unaryNegativeCodegen(builder, ctx, loc, meta);
        case PythonOpcode::UNARY_INVERT:
            return unaryInvertCodegen(builder, ctx, loc, meta);
        case PythonOpcode::FORMAT_SIMPLE:
            return formatSimpleCodegen(builder, ctx, loc, meta);
        case PythonOpcode::BUILD_STRING:
            return buildStringCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::BUILD_LIST:
            return buildListCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::LIST_EXTEND:
            return listExtendCodegen(builder, loc, instr, meta);
        case PythonOpcode::LIST_APPEND:
            return listAppendCodegen(builder, loc, instr, meta);
        case PythonOpcode::BUILD_SET:
            return buildSetCodegen(builder, ctx, loc, instr, meta);
        case PythonOpcode::SET_UPDATE:
            return setUpdateCodegen(builder, loc, instr, meta);
        case PythonOpcode::SET_ADD:
            return setAddCodegen(builder, loc, instr, meta);
        case PythonOpcode::UNKNOWN:
        default:
            throw PyCompileError("Unsupported opcode '" + pythonOpcodeToString(instr.opcode) + "'", loc);
    }
}


/**
 * Switches to writing instructions into the block denoted by the jump target instruction offset.
 *
 * Uses lookahead jump blocks to get block for the current instruction.
 */
void switchToOffsetBlock(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module,
                         const ByteCodeInstruction& instr, ConversionMeta& meta) {
    mlir::Block* targetBlock = meta.offsetToBlock[instr.offset];

    if (!builder.getBlock()->mightHaveTerminator()) {
        const mlir::Location loc = getInstructionLocation(ctx, instr, module.filename);
        // Pass current stack as block args to the target block
        llvm::SmallVector<mlir::Value> branchArgs(meta.stack.begin(), meta.stack.end());
        if (targetBlock->getNumArguments() == 0)
            for (mlir::Value v : branchArgs)
                targetBlock->addArgument(v.getType(), loc);
        builder.create<mlir::cf::BranchOp>(loc, targetBlock, mlir::ValueRange{branchArgs});
    }

    builder.setInsertionPointToStart(targetBlock);
    // Replace stack with block arguments
    meta.stack.clear();
    for (mlir::BlockArgument arg : targetBlock->getArguments())
        meta.stack.push_back(arg);
}


/**
 * Translates a ByteCodeModule to an MLIR FuncOp instruction-by-instruction
 * @param builder The MLIR OpBuilder to use by reference
 * @param ctx The MLIR Context
 * @param module The original ByteCodeModule
 * @param moduleName The name to give the MLIR module
 * @throw PyCompileError For unknown or malformed ops
 */
void buildMLIRModule(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module,
                     const std::string& moduleName) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);

    ConversionMeta meta;
    meta.isFunction = moduleName.starts_with("__pyfn_");

    mlir::FunctionType fnType;
    if (meta.isFunction)
        // Value*(Value** args, int64_t argc), matches Value::Fn
        fnType = builder.getFunctionType({pyType, pyType}, // args_ptr, argc
                                         {pyType} // return Value*
        );
    else
        // Module-level: no args, no return
        fnType = builder.getFunctionType({}, {});


    mlir::func::FuncOp fn =
            builder.create<mlir::func::FuncOp>(mlir::UnknownLoc::get(&ctx), llvm::StringRef(moduleName), fnType);

    mlir::Block* block = fn.addEntryBlock();
    builder.setInsertionPointToStart(block);

    const mlir::Location preambleLoc =
            module.instructions.empty() ? mlir::UnknownLoc::get(&ctx)
                                        : getInstructionLocation(ctx, module.instructions.front(), module.filename);

    // For functions, emit push_scope + arg unpacking preamble using block arguments
    if (meta.isFunction) {
        builder.create<pyir::PushScope>(preambleLoc);

        mlir::Value argsPtr = block->getArgument(0); // args_ptr (!pyir.object, i.e. Value**)

        for (size_t i = 0; i < static_cast<size_t>(module.info.argcount); i++) {
            mlir::IntegerAttr idxAttr = builder.getI64IntegerAttr(static_cast<int64_t>(i));
            mlir::Value argVal = builder.create<pyir::LoadArg>(preambleLoc, pyType, argsPtr, idxAttr).getResult();
            builder.create<pyir::StoreFast>(preambleLoc, module.info.varnames[i], argVal);
        }
    }

    // Exception table handler targets also get blocks
    for (const ExceptionTableEntry& e : module.info.exceptionTable)
        if (!meta.offsetToBlock.contains(e.target))
            meta.offsetToBlock[e.target] = fn.addBlock();

    // Pre-pass: collect all jump target offsets and create blocks for them.
    // This must happen before emission so forward jumps can reference blocks that haven't been emitted yet.
    for (const ByteCodeInstruction& instr : module.instructions) {
        const int64_t* target = std::get_if<int64_t>(&instr.argval);
        const bool isJumpTarget = instr.opcode == PythonOpcode::JUMP_FORWARD ||
                                  instr.opcode == PythonOpcode::POP_JUMP_IF_TRUE ||
                                  instr.opcode == PythonOpcode::POP_JUMP_IF_FALSE;
        if (target && isJumpTarget && !meta.offsetToBlock.contains(*target))
            meta.offsetToBlock[*target] = fn.addBlock();
    }

    for (const ByteCodeInstruction& instr : module.instructions) {
        // If this offset is a jump target, switch to its block.
        // Emit a branch from the current block if it isn't already terminated.
        if (meta.offsetToBlock.contains(instr.offset))
            switchToOffsetBlock(builder, ctx, module, instr, meta);

        buildMLIRInstruction(builder, ctx, module, fn, instr, meta);
    }
}


/**
 * Inserts a main function as an explicit entrypoint to the module.
 */
void insertMainEntryPoint(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, llvm::StringRef moduleFnName) {
    const mlir::UnknownLoc loc = mlir::UnknownLoc::get(&ctx);
    mlir::IntegerType i32Type = builder.getIntegerType(32);

    // main returns int
    mlir::FunctionType mainType = builder.getFunctionType({}, {i32Type});
    mlir::func::FuncOp mainFn = builder.create<mlir::func::FuncOp>(loc, "main", mainType);

    mlir::Block* block = mainFn.addEntryBlock();
    builder.setInsertionPointToStart(block);

    // Call the compiled module function
    builder.create<mlir::func::CallOp>(loc, moduleFnName, mlir::TypeRange{});

    // return 0
    mlir::arith::ConstantIntOp zero = builder.create<mlir::arith::ConstantIntOp>(loc, 0, 32);
    builder.create<mlir::func::ReturnOp>(loc, mlir::ValueRange{zero});
}


mlir::OwningOpRef<mlir::ModuleOp> generatePyIR(mlir::MLIRContext& ctx, const ByteCodeModule& module) {
    // Dialects must be loaded before any ops are created
    ctx.loadDialect<pyir::PyIRDialect>();
    ctx.loadDialect<mlir::func::FuncDialect>();
    ctx.loadDialect<mlir::cf::ControlFlowDialect>();
    ctx.loadDialect<mlir::arith::ArithDialect>();

    const std::string baseModuleName = std::filesystem::path(module.filename).filename();
    mlir::OpBuilder builder(&ctx);
    const mlir::FileLineColLoc fileLoc = mlir::FileLineColLoc::get(&ctx, baseModuleName, 0, 0);
    mlir::ModuleOp mlirModule = mlir::ModuleOp::create(fileLoc);
    builder.setInsertionPointToEnd(mlirModule.getBody());

    const std::string mlirModuleName = mangleModuleName(baseModuleName);
    buildMLIRModule(builder, ctx, module, mlirModuleName);
    // Reset insertion point to module level before emitting main
    builder.setInsertionPointToEnd(mlirModule.getBody());
    insertMainEntryPoint(builder, ctx, mlirModuleName);

    // Verify the module is well-formed before returning
    if (mlir::failed(mlir::verify(mlirModule))) {
        mlirModule.getOperation()->print(llvm::errs());
        throw std::runtime_error(module.moduleName + ": error: MLIR module verification failed");
    }

    return mlirModule;
}


mlir::OwningOpRef<mlir::ModuleOp> mergePyIRModules(mlir::MLIRContext& ctx,
                                                   std::vector<mlir::OwningOpRef<mlir::ModuleOp>>& mlirModules) {
    if (mlirModules.empty())
        throw std::runtime_error("Cannot merge an empty module list");

    mlir::Location loc = mlirModules[0].get().getLoc();
    std::string mlirModuleName;
    if (const mlir::FileLineColLoc firstFileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(loc))
        mlirModuleName = firstFileLoc.getFilename().str();
    else
        throw std::runtime_error("Unable to parse merged PyIR module name");

    const mlir::FileLineColLoc fileLoc = mlir::FileLineColLoc::get(&ctx, mlirModuleName, 0, 0);
    mlir::ModuleOp merged = mlir::ModuleOp::create(fileLoc);

    for (mlir::OwningOpRef<mlir::ModuleOp>& mlirModule : mlirModules) {
        llvm::SmallVector<mlir::Operation*> ops;
        for (mlir::Operation& op : mlirModule.get().getBody()->getOperations())
            ops.push_back(&op);

        for (mlir::Operation* op : ops) {
            op->remove();
            merged.getBody()->push_back(op);
        }
    }

    return merged;
}
