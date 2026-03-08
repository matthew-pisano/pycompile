//
// Created by matthew on 3/6/26.
//

#include "pyir/pyir_codegen.h"

#include <mlir/IR/Verifier.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>

#include "pyir/pyir_ops.h"


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
 * Resolves free or cell variable names based on the given CodeInfo
 * @param info The CodeInfo of the program
 * @param idx The index of the desired name
 * @return The name of the free or cell variable
 */
std::string resolve_deref(const CodeInfo& info, const int idx) {
    const int cellStart = static_cast<int>(info.varnames.size());
    const int freeStart = cellStart + static_cast<int>(info.cellvars.size());
    if (idx < cellStart)
        return info.varnames[idx];
    if (idx < freeStart)
        return info.cellvars[idx - cellStart];
    return info.freevars[idx - freeStart];
}


/**
 * Translates a ByteCodeModule to an MLIR FuncOp instruction-by-instruction
 * @param builder The MLIR OpBuilder to use
 * @param ctx The MLIR Context
 * @param module The original ByteCodeModule
 * @return The result MLIR FuncOp
 * @throw runtime_error For unknown or malformed ops
 */
mlir::func::FuncOp emitFuncOps(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module) {
    pyir::ByteCodeObjectType pyType = pyir::ByteCodeObjectType::get(&ctx);

    // All Python module-level code is wrapped in a zero-argument function that returns a single PyObject (the final return value).
    mlir::FunctionType fnType = builder.getFunctionType({}, {pyType});
    mlir::func::FuncOp fn = builder.create<mlir::func::FuncOp>(mlir::UnknownLoc::get(&ctx),
                                                               llvm::StringRef(module.moduleName),
                                                               fnType);

    mlir::Block* block = fn.addEntryBlock();
    builder.setInsertionPointToStart(block);

    // Value stack — maps the CPython evaluation stack to SSA values.
    std::vector<mlir::Value> stack;

    // Pre-pass: collect all jump target offsets and create blocks for them.
    // This must happen before emission so forward jumps can reference blocks that haven't been emitted yet.
    std::unordered_map<size_t, mlir::Block*> offsetToBlock;
    for (const ByteCodeInstruction& instr : module.instructions) {
        const int* target = std::get_if<int>(&instr.argval);
        const bool isJumpTarget = instr.opcode == PythonOpcode::JUMP_FORWARD ||
                                  instr.opcode == PythonOpcode::POP_JUMP_IF_TRUE ||
                                  instr.opcode == PythonOpcode::POP_JUMP_IF_FALSE;
        if (target && isJumpTarget && !offsetToBlock.contains(*target))
            offsetToBlock[*target] = fn.addBlock();
    }

    // Exception table handler targets also get blocks
    for (const ExceptionTableEntry& e : module.info.exceptionTable)
        if (!offsetToBlock.contains(e.target))
            offsetToBlock[e.target] = fn.addBlock();

    for (const ByteCodeInstruction& instr : module.instructions) {
        mlir::Location loc = getInstructionLocation(ctx, instr, module.filename);

        // If this offset is a jump target, switch to its block.
        // Emit a branch from the current block if it isn't already terminated.
        if (offsetToBlock.contains(instr.offset)) {
            mlir::Block* targetBlock = offsetToBlock[instr.offset];
            if (!builder.getBlock()->mightHaveTerminator())
                builder.create<mlir::cf::BranchOp>(loc, targetBlock);
            builder.setInsertionPointToStart(targetBlock);
        }

        switch (instr.opcode) {
            case PythonOpcode::RESUME:
                // Bookkeeping instruction, no MLIR equivalent needed.
                break;

            case PythonOpcode::LOAD_CONST: {
                mlir::Attribute attr;
                if (const std::string* s = std::get_if<std::string>(&instr.argval))
                    attr = builder.getStringAttr(*s);
                else if (const int* i = std::get_if<int>(&instr.argval))
                    attr = builder.getI64IntegerAttr(*i);
                else
                    // None or other unhandled constant — use unit attr as a placeholder
                    attr = builder.getUnitAttr();

                stack.push_back(builder.create<pyir::LoadConst>(loc, pyType, attr).getResult());
                break;
            }

            case PythonOpcode::LOAD_NAME: {
                const std::string* name = std::get_if<std::string>(&instr.argval);
                if (!name)
                    throw std::runtime_error("LOAD_NAME must have a string argval");
                stack.push_back(builder.create<pyir::LoadName>(loc, pyType, *name).getResult());
                break;
            }

            case PythonOpcode::LOAD_FAST: {
                const std::string* name = std::get_if<std::string>(&instr.argval);
                if (!name)
                    throw std::runtime_error("LOAD_FAST must have a string argval");
                stack.push_back(builder.create<pyir::LoadFast>(loc, pyType, *name).getResult());
                break;
            }

            case PythonOpcode::STORE_FAST: {
                const std::string* name = std::get_if<std::string>(&instr.argval);
                if (!name)
                    throw std::runtime_error("STORE_FAST must have a string argval");
                mlir::Value val = stack.back();
                stack.pop_back();
                builder.create<pyir::StoreFast>(loc, *name, val);
                break;
            }

            case PythonOpcode::LOAD_DEREF: {
                const int* idx = std::get_if<int>(&instr.argval);
                if (!idx)
                    throw std::runtime_error("LOAD_DEREF must have an int argval");
                const std::string name = resolve_deref(module.info, *idx);
                stack.push_back(builder.create<pyir::LoadDeref>(loc, pyType, name).getResult());
                break;
            }

            case PythonOpcode::STORE_DEREF: {
                const int* idx = std::get_if<int>(&instr.argval);
                if (!idx)
                    throw std::runtime_error("STORE_DEREF must have an int argval");
                const std::string name = resolve_deref(module.info, *idx);
                mlir::Value val = stack.back();
                stack.pop_back();
                builder.create<pyir::StoreDeref>(loc, name, val);
                break;
            }

            case PythonOpcode::BINARY_OP: {
                const std::string* opStr = std::get_if<std::string>(&instr.argval);
                if (!opStr)
                    throw std::runtime_error("BINARY_OP must have a string argval");
                mlir::Value rhs = stack.back();
                stack.pop_back();
                mlir::Value lhs = stack.back();
                stack.pop_back();
                stack.push_back(builder.create<pyir::BinaryOp>(loc, pyType, *opStr, lhs, rhs).getResult());
                break;
            }

            case PythonOpcode::PUSH_NULL:
                // Push a null sentinel onto the stack for the call convention.
                stack.push_back(builder.create<pyir::PushNull>(loc, pyType).getResult());
                break;

            case PythonOpcode::CALL: {
                const int* argc = std::get_if<int>(&instr.argval);
                if (!argc)
                    throw std::runtime_error("CALL must have an int argval");
                // Pop arguments in reverse order
                std::vector<mlir::Value> args(*argc);
                for (int i = *argc - 1; i >= 0; i--) {
                    args[i] = stack.back();
                    stack.pop_back();
                }

                // Pop callee, then null sentinel below it
                const mlir::Value callee = stack.back();
                stack.pop_back();
                stack.pop_back(); // null sentinel from PUSH_NULL

                stack.push_back(builder.create<pyir::Call>(loc, pyType, callee, args).getResult());
                break;
            }

            case PythonOpcode::POP_TOP:
                // Discard top of stack — if the value is unused MLIR's DCE will clean up the producing op if it's Pure.
                stack.pop_back();
                break;

            case PythonOpcode::RETURN_VALUE: {
                const mlir::Value val = stack.back();
                stack.pop_back();
                builder.create<mlir::func::ReturnOp>(loc, mlir::ValueRange{val});
                break;
            }

            case PythonOpcode::JUMP_FORWARD: {
                const int* target = std::get_if<int>(&instr.argval);
                if (!target)
                    throw std::runtime_error("JUMP_FORWARD must have an int argval");
                mlir::Block* dest = offsetToBlock.at(*target);
                builder.create<mlir::cf::BranchOp>(loc, dest);
                break;
            }

            case PythonOpcode::POP_JUMP_IF_TRUE: {
                const int* target = std::get_if<int>(&instr.argval);
                if (!target)
                    throw std::runtime_error("POP_JUMP_IF_TRUE must have an int argval");
                mlir::Value cond = stack.back();
                stack.pop_back();
                mlir::Block* trueBlock = offsetToBlock.at(*target);
                mlir::Block* falseBlock = fn.addBlock(); // fall-through block
                builder.create<mlir::cf::CondBranchOp>(loc, cond, trueBlock, falseBlock);
                builder.setInsertionPointToStart(falseBlock);
                break;
            }

            case PythonOpcode::POP_JUMP_IF_FALSE: {
                const int* target = std::get_if<int>(&instr.argval);
                if (!target)
                    throw std::runtime_error("POP_JUMP_IF_FALSE must have an int argval");
                mlir::Value cond = stack.back();
                stack.pop_back();
                mlir::Block* falseBlock = offsetToBlock.at(*target);
                mlir::Block* trueBlock = fn.addBlock(); // fall-through block
                builder.create<mlir::cf::CondBranchOp>(loc, cond, trueBlock, falseBlock);
                builder.setInsertionPointToStart(trueBlock);
                break;
            }

            case PythonOpcode::UNKNOWN:
            default:
                throw std::runtime_error(
                        "Unhandled opcode: " + std::string(pythonOpcodeToString(instr.opcode)) + " at offset " +
                        std::to_string(instr.offset));
        }
    }

    return fn;
}


mlir::OwningOpRef<mlir::ModuleOp> generateMLIR(mlir::MLIRContext& ctx, const ByteCodeModule& module) {
    // Dialects must be loaded before any ops are created
    ctx.loadDialect<pyir::PyIRDialect>();
    ctx.loadDialect<mlir::func::FuncDialect>();
    ctx.loadDialect<mlir::cf::ControlFlowDialect>();

    mlir::OpBuilder builder(&ctx);
    mlir::ModuleOp mlirModule = mlir::ModuleOp::create(mlir::UnknownLoc::get(&ctx));
    builder.setInsertionPointToEnd(mlirModule.getBody());

    const mlir::func::FuncOp fn = emitFuncOps(builder, ctx, module);
    mlirModule.push_back(fn);

    // Verify the module is well-formed before returning
    if (mlir::failed(mlir::verify(mlirModule))) {
        mlirModule.emitError("generated module failed verification");
        throw std::runtime_error("MLIR verification failed for module: " + module.moduleName);
    }

    return mlirModule;
}
