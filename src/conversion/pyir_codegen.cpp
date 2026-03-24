//
// Created by matthew on 3/6/26.
//

#include "conversion/pyir_codegen.h"

#include <filesystem>
#include <iostream>
#include <mlir/IR/Verifier.h>
#include <mlir/IR/AsmState.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>

#include "utils.h"
#include "pyir/pyir_ops.h"
#include "pyir/pyir_attrs.h"


// Global counter to build unique function names
static size_t fnCounter = 0;

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
 * Resolves free or cell variable names based on the given CodeInfo
 * @param info The CodeInfo of the program
 * @param idx The index of the desired name
 * @return The name of the free or cell variable
 */
std::string resolve_deref(const CodeInfo& info, const int64_t idx) {
    const int cellS_start = static_cast<int>(info.varnames.size());
    const int freeStart = cellS_start + static_cast<int>(info.cellvars.size());
    if (idx < cellS_start)
        return info.varnames[idx];
    if (idx < freeStart)
        return info.cellvars[idx - cellS_start];
    return info.freevars[idx - freeStart];
}


/**
 * A collection of data structures used for bookkeeping during the bytecode to MLIR conversion process
 */
struct ConversionMeta {
    /// Value stack, maps the CPython evaluation stack to SSA values.
    std::vector<mlir::Value> stack;
    /// Maps program offsets to program blocks, used for jump addressing.
    std::unordered_map<size_t, mlir::Block*> offsetToBlock;
    /// Keeps track of declared function names before construction.
    std::unordered_map<mlir::Value*, std::string> pendingFunctions;
    /// Whether the module is a top-level module or a function
    bool isFunction;
};


// Forward declaration
void buildMLIRModule(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module,
                     const std::string& moduleName);


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
            break;

        case PythonOpcode::MAKE_FUNCTION: {
            mlir::Value sentinel = meta.stack.back();
            meta.stack.pop_back();

            const std::string fnName = meta.pendingFunctions.at(
                    static_cast<mlir::Value*>(sentinel.getAsOpaquePointer()));
            meta.pendingFunctions.erase(static_cast<mlir::Value*>(sentinel.getAsOpaquePointer()));
            // Erase the sentinel PushNull op since it was just a placeholder
            sentinel.getDefiningOp()->erase();

            meta.stack.push_back(builder.create<pyir::MakeFunction>(loc, pyType, fnName).getResult());
            break;
        }

        case PythonOpcode::LOAD_GLOBAL: {
            const std::string* name = std::get_if<std::string>(&instr.argval);
            if (!name)
                throw PyCompileError("LOAD_GLOBAL must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            // Push the value
            meta.stack.push_back(builder.create<pyir::LoadName>(loc, pyType, *name).getResult());
            // LOAD_GLOBAL also pushes a null sentinel
            meta.stack.push_back(builder.create<pyir::PushNull>(loc, pyType).getResult());
            break;
        }

        case PythonOpcode::LOAD_FAST_BORROW: {
            const std::string* name = std::get_if<std::string>(&instr.argval);
            if (!name)
                throw PyCompileError("LOAD_FAST_BORROW must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            meta.stack.push_back(builder.create<pyir::LoadFast>(loc, pyType, *name).getResult());
            break;
        }

        case PythonOpcode::LOAD_CONST: {
            if (std::holds_alternative<ArgvalNone>(instr.argval)) {
                // None is a valid return value inside functions
                if (meta.isFunction) {
                    mlir::Attribute attr = pyir::NoneAttr::get(&ctx);
                    meta.stack.push_back(builder.create<pyir::LoadConst>(loc, pyType, attr).getResult());
                }
                break;
            }

            // Nested code object: emit a child FuncOp and push its name for MAKE_FUNCTION
            if (const auto* nestedPtr = std::get_if<std::shared_ptr<ByteCodeModule> >(&instr.argval)) {
                const ByteCodeModule& nested = **nestedPtr;

                // Mangle a unique name: __pyfn_<funcname>_<counter>
                const std::string fnName = "__pyfn_" + nested.info.codeName + "_" + std::to_string(fnCounter++);

                // Save insertion point, emit the nested FuncOp at module level, then restore
                mlir::OpBuilder::InsertionGuard guard(builder);
                mlir::ModuleOp parentModule = fn->getParentOfType<mlir::ModuleOp>();
                builder.setInsertionPointToEnd(parentModule.getBody());
                buildMLIRModule(builder, ctx, nested, fnName);

                mlir::Value sentinel = builder.create<pyir::PushNull>(loc, pyType).getResult();
                meta.pendingFunctions[static_cast<mlir::Value*>(sentinel.getAsOpaquePointer())] = fnName;
                meta.stack.push_back(sentinel);
                break;
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
            break;
        }

        case PythonOpcode::LOAD_NAME: {
            const std::string* name = std::get_if<std::string>(&instr.argval);
            if (!name)
                throw PyCompileError("LOAD_NAME must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            meta.stack.push_back(builder.create<pyir::LoadName>(loc, pyType, *name).getResult());
            break;
        }

        case PythonOpcode::STORE_NAME: {
            const std::string* name = std::get_if<std::string>(&instr.argval);
            if (!name)
                throw PyCompileError("STORE_NAME must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Value val = meta.stack.back();
            meta.stack.pop_back();
            builder.create<pyir::StoreName>(loc, *name, val);
            break;
        }

        case PythonOpcode::LOAD_FAST: {
            const std::string* name = std::get_if<std::string>(&instr.argval);
            if (!name)
                throw PyCompileError("LOAD_FAST must have a string argvall",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            meta.stack.push_back(builder.create<pyir::LoadFast>(loc, pyType, *name).getResult());
            break;
        }

        case PythonOpcode::STORE_FAST: {
            const std::string* name = std::get_if<std::string>(&instr.argval);
            if (!name)
                throw PyCompileError("STORE_FAST must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Value val = meta.stack.back();
            meta.stack.pop_back();
            builder.create<pyir::StoreFast>(loc, *name, val);
            break;
        }

        case PythonOpcode::LOAD_DEREF: {
            const int64_t* idx = std::get_if<int64_t>(&instr.argval);
            if (!idx)
                throw PyCompileError("LOAD_DEREF must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            const std::string name = resolve_deref(module.info, *idx);
            meta.stack.push_back(builder.create<pyir::LoadDeref>(loc, pyType, name).getResult());
            break;
        }

        case PythonOpcode::STORE_DEREF: {
            const int64_t* idx = std::get_if<int64_t>(&instr.argval);
            if (!idx)
                throw PyCompileError("STORE_DEREF must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            const std::string name = resolve_deref(module.info, *idx);
            mlir::Value val = meta.stack.back();
            meta.stack.pop_back();
            builder.create<pyir::StoreDeref>(loc, name, val);
            break;
        }

        case PythonOpcode::BINARY_OP: {
            const std::string opStr = instr.argrepr;
            if (opStr.empty())
                throw PyCompileError("BINARY_OP must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Value rhs = meta.stack.back();
            meta.stack.pop_back();
            mlir::Value lhs = meta.stack.back();
            meta.stack.pop_back();
            meta.stack.push_back(builder.create<pyir::BinaryOp>(loc, pyType, opStr, lhs, rhs).getResult());
            break;
        }

        case PythonOpcode::COMPARE_OP: {
            const std::string opStr = instr.argrepr;
            if (opStr.empty())
                throw PyCompileError("COMPARE_OP must have a string argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Value rhs = meta.stack.back();
            meta.stack.pop_back();
            mlir::Value lhs = meta.stack.back();
            meta.stack.pop_back();
            meta.stack.push_back(builder.create<pyir::CompareOp>(loc, pyType, opStr, lhs, rhs).getResult());
            break;
        }

        case PythonOpcode::PUSH_NULL:
            // Push a null sentinel onto the stack for the call convention.
            meta.stack.push_back(builder.create<pyir::PushNull>(loc, pyType).getResult());
            break;

        case PythonOpcode::CALL: {
            const int64_t* argc = std::get_if<int64_t>(&instr.argval);
            if (!argc)
                throw PyCompileError("CALL must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
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
            break;
        }

        case PythonOpcode::POP_TOP:
            // Discard top of stack, if the value is unused MLIR's DCE will clean up the producing op if it's Pure.
            meta.stack.pop_back();
            break;

        case PythonOpcode::RETURN_VALUE: {
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
            break;
        }

        case PythonOpcode::COPY: {
            const int64_t* copyIdx = std::get_if<int64_t>(&instr.argval);
            auto value = meta.stack.at(meta.stack.size() - *copyIdx);
            meta.stack.push_back(value);
            break;
        }

        case PythonOpcode::JUMP_FORWARD: {
            const int64_t* target = std::get_if<int64_t>(&instr.argval);
            if (!target)
                throw PyCompileError("JUMP_FORWARD must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Block* dest = meta.offsetToBlock.at(*target);
            builder.create<mlir::cf::BranchOp>(loc, dest);
            break;
        }

        case PythonOpcode::POP_JUMP_IF_TRUE: {
            const int64_t* target = std::get_if<int64_t>(&instr.argval);
            if (!target)
                throw PyCompileError("POP_JUMP_IF_TRUE must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Value cond = meta.stack.back();
            meta.stack.pop_back();

            // unbox !pyir.object -> i1
            mlir::Value i1cond = builder.create<pyir::IsTruthy>(loc, builder.getI1Type(), cond).getResult();
            mlir::Block* trueBlock = meta.offsetToBlock.at(*target);
            mlir::Block* falseBlock = fn.addBlock();

            // Pass remaining stack values as block arguments to the true block so they are available after the jump
            llvm::SmallVector<mlir::Value> trueArgs(meta.stack.begin(), meta.stack.end());
            llvm::SmallVector<mlir::Type> trueArgTypes;
            for (auto v : trueArgs)
                trueArgTypes.push_back(v.getType());

            if (trueBlock->getNumArguments() == 0)
                for (auto t : trueArgTypes)
                    trueBlock->addArgument(t, loc);

            builder.create<mlir::cf::CondBranchOp>(loc, i1cond, trueBlock, mlir::ValueRange{trueArgs},
                                                   falseBlock, mlir::ValueRange{});
            builder.setInsertionPointToStart(falseBlock);
            break;
        }

        case PythonOpcode::POP_JUMP_IF_FALSE: {
            const int64_t* target = std::get_if<int64_t>(&instr.argval);
            if (!target)
                throw PyCompileError("POP_JUMP_IF_FALSE must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Value cond = meta.stack.back();
            meta.stack.pop_back();

            // unbox !pyir.object -> i1
            mlir::Value i1cond = builder.create<pyir::IsTruthy>(loc, builder.getI1Type(), cond).getResult();
            mlir::Block* falseBlock = meta.offsetToBlock.at(*target);
            mlir::Block* trueBlock = fn.addBlock();

            // Pass remaining stack values as block arguments to the false block so they are available after the jump
            llvm::SmallVector<mlir::Value> falseArgs(meta.stack.begin(), meta.stack.end());
            llvm::SmallVector<mlir::Type> falseArgTypes;
            for (mlir::Value v : falseArgs)
                falseArgTypes.push_back(v.getType());

            // Add block arguments to falseBlock if not already added
            if (falseBlock->getNumArguments() == 0)
                for (mlir::Type t : falseArgTypes)
                    falseBlock->addArgument(t, loc);

            builder.create<mlir::cf::CondBranchOp>(loc, i1cond, trueBlock, mlir::ValueRange{}, falseBlock,
                                                   mlir::ValueRange{falseArgs});
            builder.setInsertionPointToStart(trueBlock);
            break;
        }

        case PythonOpcode::LOAD_SMALL_INT: {
            const int64_t* target = std::get_if<int64_t>(&instr.argval);
            if (!target)
                throw PyCompileError("LOAD_SMALL_INT must have an int argval",
                                     std::filesystem::path(module.filename).filename(),
                                     instr.lineno, instr.offset);
            mlir::Attribute attr = builder.getI64IntegerAttr(*target);
            meta.stack.push_back(builder.create<pyir::LoadConst>(loc, pyType, attr).getResult());
            break;
        }

        case PythonOpcode::TO_BOOL: {
            mlir::Value toConvert = meta.stack.back();
            meta.stack.pop_back(); // Replace value
            meta.stack.push_back(builder.create<pyir::ToBool>(loc, pyType, toConvert).getResult());
            break;
        }

        case PythonOpcode::UNARY_NOT: {
            mlir::Value value = meta.stack.back();
            meta.stack.pop_back();
            meta.stack.push_back(builder.create<pyir::UnaryNot>(loc, pyType, value).getResult());
            break;
        }
        case PythonOpcode::UNARY_NEGATIVE: {
            mlir::Value value = meta.stack.back();
            meta.stack.pop_back();
            meta.stack.push_back(builder.create<pyir::UnaryNegative>(loc, pyType, value).getResult());
            break;
        }

        case PythonOpcode::UNARY_INVERT: {
            mlir::Value value = meta.stack.back();
            meta.stack.pop_back();
            meta.stack.push_back(builder.create<pyir::UnaryInvert>(loc, pyType, value).getResult());
            break;
        }

        case PythonOpcode::FORMAT_SIMPLE: {
            mlir::Value val = meta.stack.back();
            meta.stack.pop_back();
            meta.stack.push_back(builder.create<pyir::FormatSimple>(loc, pyType, val).getResult());
            break;
        }
        case PythonOpcode::BUILD_STRING: {
            const int64_t* count = std::get_if<int64_t>(&instr.argval);
            if (!count)
                throw std::runtime_error("BUILD_STRING must have an int argval");

            std::vector<mlir::Value> parts(*count);
            for (int64_t i = *count - 1; i >= 0; i--) {
                parts[i] = meta.stack.back();
                meta.stack.pop_back();
            }
            meta.stack.push_back(builder.create<pyir::BuildString>(loc, pyType, parts).getResult());
            break;
        }

        case PythonOpcode::UNKNOWN:
        default: {
            throw PyCompileError("Unsupported opcode '" + pythonOpcodeToString(instr.opcode) + "'",
                                 std::filesystem::path(module.filename).filename(),
                                 instr.lineno, instr.offset);
        }
    }
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
        fnType = builder.getFunctionType(
                {pyType, pyType}, // args_ptr, argc
                {pyType} // return Value*
                );
    else
        // Module-level: no args, no return
        fnType = builder.getFunctionType({}, {});


    mlir::func::FuncOp fn = builder.create<mlir::func::FuncOp>(mlir::UnknownLoc::get(&ctx),
                                                               llvm::StringRef(moduleName), fnType);

    mlir::Block* block = fn.addEntryBlock();
    builder.setInsertionPointToStart(block);

    const mlir::Location preambleLoc = module.instructions.empty()
                                           ? mlir::UnknownLoc::get(&ctx)
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
        if (meta.offsetToBlock.contains(instr.offset)) {
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

        buildMLIRInstruction(builder, ctx, module, fn, instr, meta);
    }

}


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
                                                   std::vector<mlir::OwningOpRef<mlir::ModuleOp> >& mlirModules) {
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
