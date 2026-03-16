//
// Created by matthew on 3/6/26.
//

#include "pyir/pyir_codegen.h"

#include <filesystem>
#include <iostream>
#include <mlir/IR/Verifier.h>
#include <mlir/IR/AsmState.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <mlir/Dialect/Arith/IR/Arith.h>

#include "pyir/pyir_ops.h"

namespace pyir {

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
    std::string resolve_deref(const CodeInfo& info, const int idx) {
        const int cellS_start = static_cast<int>(info.varnames.size());
        const int freeStart = cellS_start + static_cast<int>(info.cellvars.size());
        if (idx < cellS_start)
            return info.varnames[idx];
        if (idx < freeStart)
            return info.cellvars[idx - cellS_start];
        return info.freevars[idx - freeStart];
    }


    /**
     * Translates a ByteCodeModule to an MLIR FuncOp instruction-by-instruction
     * @param builder The MLIR OpBuilder to use by reference
     * @param ctx The MLIR Context
     * @param module The original ByteCodeModule
     * @param moduleName The name to give the MLIR module
     * @throw runtime_error For unknown or malformed ops
     */
    void buildMLIRModule(mlir::OpBuilder& builder, mlir::MLIRContext& ctx, const ByteCodeModule& module,
                         const std::string& moduleName) {
        ByteCodeObjectType pyType = ByteCodeObjectType::get(&ctx);

        // All Python module-level code is wrapped in a zero-argument function that returns a single PyObject (the final return value).
        mlir::FunctionType fnType = builder.getFunctionType({}, {});
        mlir::func::FuncOp fn = builder.create<mlir::func::FuncOp>(mlir::UnknownLoc::get(&ctx),
                                                                   llvm::StringRef(moduleName),
                                                                   fnType);

        mlir::Block* block = fn.addEntryBlock();
        builder.setInsertionPointToStart(block);

        // Value stack, maps the CPython evaluation stack to SSA values.
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
            const mlir::Location loc = getInstructionLocation(ctx, instr, module.filename);

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
                    if (std::holds_alternative<ArgvalNone>(instr.argval))
                        break;

                    mlir::Attribute attr;
                    if (const std::string* s = std::get_if<std::string>(&instr.argval))
                        attr = builder.getStringAttr(*s);
                    else if (const int* i = std::get_if<int>(&instr.argval))
                        attr = builder.getI64IntegerAttr(*i);

                    stack.push_back(builder.create<LoadConst>(loc, pyType, attr).getResult());
                    break;
                }

                case PythonOpcode::LOAD_NAME: {
                    const std::string* name = std::get_if<std::string>(&instr.argval);
                    if (!name)
                        throw std::runtime_error("LOAD_NAME must have a string argval");
                    stack.push_back(builder.create<LoadName>(loc, pyType, *name).getResult());
                    break;
                }

                case PythonOpcode::STORE_NAME: {
                    const std::string* name = std::get_if<std::string>(&instr.argval);
                    if (!name)
                        throw std::runtime_error("STORE_FAST must have a string argval");
                    mlir::Value val = stack.back();
                    stack.pop_back();
                    builder.create<StoreName>(loc, *name, val);
                    break;
                }

                case PythonOpcode::LOAD_FAST: {
                    const std::string* name = std::get_if<std::string>(&instr.argval);
                    if (!name)
                        throw std::runtime_error("LOAD_FAST must have a string argval");
                    stack.push_back(builder.create<LoadFast>(loc, pyType, *name).getResult());
                    break;
                }

                case PythonOpcode::STORE_FAST: {
                    const std::string* name = std::get_if<std::string>(&instr.argval);
                    if (!name)
                        throw std::runtime_error("STORE_FAST must have a string argval");
                    mlir::Value val = stack.back();
                    stack.pop_back();
                    builder.create<StoreFast>(loc, *name, val);
                    break;
                }

                case PythonOpcode::LOAD_DEREF: {
                    const int* idx = std::get_if<int>(&instr.argval);
                    if (!idx)
                        throw std::runtime_error("LOAD_DEREF must have an int argval");
                    const std::string name = resolve_deref(module.info, *idx);
                    stack.push_back(builder.create<LoadDeref>(loc, pyType, name).getResult());
                    break;
                }

                case PythonOpcode::STORE_DEREF: {
                    const int* idx = std::get_if<int>(&instr.argval);
                    if (!idx)
                        throw std::runtime_error("STORE_DEREF must have an int argval");
                    const std::string name = resolve_deref(module.info, *idx);
                    mlir::Value val = stack.back();
                    stack.pop_back();
                    builder.create<StoreDeref>(loc, name, val);
                    break;
                }

                case PythonOpcode::BINARY_OP: {
                    const std::string opStr = instr.argrepr;
                    if (opStr.empty())
                        throw std::runtime_error("BINARY_OP must have a string argval");
                    mlir::Value rhs = stack.back();
                    stack.pop_back();
                    mlir::Value lhs = stack.back();
                    stack.pop_back();
                    stack.push_back(builder.create<BinaryOp>(loc, pyType, opStr, lhs, rhs).getResult());
                    break;
                }

                case PythonOpcode::COMPARE_OP: {
                    const std::string opStr = instr.argrepr;
                    if (opStr.empty())
                        throw std::runtime_error("COMPARE_OP must have a string argval");
                    mlir::Value rhs = stack.back();
                    stack.pop_back();
                    mlir::Value lhs = stack.back();
                    stack.pop_back();
                    stack.push_back(builder.create<CompareOp>(loc, pyType, opStr, lhs, rhs).getResult());
                    break;
                }

                case PythonOpcode::PUSH_NULL:
                    // Push a null sentinel onto the stack for the call convention.
                    stack.push_back(builder.create<PushNull>(loc, pyType).getResult());
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

                    stack.pop_back(); // null sentinel
                    mlir::Value callee = stack.back();
                    stack.pop_back(); // actual callee

                    stack.push_back(builder.create<Call>(loc, pyType, callee, args).getResult());
                    break;
                }

                case PythonOpcode::POP_TOP:
                    // Discard top of stack, if the value is unused MLIR's DCE will clean up the producing op if it's Pure.
                    stack.pop_back();
                    break;

                case PythonOpcode::RETURN_VALUE: {
                    if (!stack.empty())
                        stack.pop_back();
                    builder.create<mlir::func::ReturnOp>(loc);
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

                case PythonOpcode::LOAD_SMALL_INT: {
                    const int* target = std::get_if<int>(&instr.argval);
                    if (!target)
                        throw std::runtime_error("LOAD_SMALL_INT must have an int argval");
                    mlir::Attribute attr = builder.getI64IntegerAttr(*target);
                    stack.push_back(builder.create<LoadConst>(loc, pyType, attr).getResult());
                    break;
                }

                case PythonOpcode::TO_BOOL: {
                    mlir::Value toConvert = stack.back();
                    stack.pop_back(); // Replace value
                    stack.push_back(builder.create<ToBool>(loc, pyType, toConvert).getResult());
                    break;
                }

                case PythonOpcode::UNARY_NOT: {
                    mlir::Value value = stack.back();
                    stack.pop_back();
                    stack.push_back(builder.create<UnaryNot>(loc, pyType, value).getResult());
                    break;
                }
                case PythonOpcode::UNARY_NEGATIVE: {
                    mlir::Value value = stack.back();
                    stack.pop_back();
                    stack.push_back(builder.create<UnaryNegative>(loc, pyType, value).getResult());
                    break;
                }

                case PythonOpcode::UNARY_INVERT: {
                    mlir::Value value = stack.back();
                    stack.pop_back();
                    stack.push_back(builder.create<UnaryInvert>(loc, pyType, value).getResult());
                    break;
                }

                case PythonOpcode::UNKNOWN:
                default:
                    throw std::runtime_error(
                            "Unhandled opcode: " + std::string(pythonOpcodeToString(instr.opcode)) + " at offset " +
                            std::to_string(instr.offset));
            }
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
        auto zero = builder.create<mlir::arith::ConstantIntOp>(loc, 0, 32);
        builder.create<mlir::func::ReturnOp>(loc, mlir::ValueRange{zero});
    }


    mlir::OwningOpRef<mlir::ModuleOp> generatePyIR(mlir::MLIRContext& ctx, const ByteCodeModule& module) {
        // Dialects must be loaded before any ops are created
        ctx.loadDialect<PyIRDialect>();
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
            mlirModule.emitError("generated module failed verification");
            throw std::runtime_error("MLIR verification failed for module: " + module.moduleName);
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


    /**
     * A bridge between llvm::raw_ostream and std::ostream
     */
    class OstreamBridge : public llvm::raw_ostream {
    public:
        explicit OstreamBridge(std::ostream& os) :
            os_(os), pos_(0) {
        }

        void write_impl(const char* ptr, const size_t size) override {
            os_.write(ptr, static_cast<std::streamsize>(size));
            pos_ += size;
        }

        [[nodiscard]] uint64_t current_pos() const override { return pos_; }

    private:
        std::ostream& os_;
        uint64_t pos_;
    };


    void printMLIRFuncOp(mlir::func::FuncOp fn, std::ostream& os) {
        OstreamBridge bridge(os);
        fn.print(bridge);
        bridge.flush();
        os << std::endl;
    }


    void serializePyIRModule(const mlir::ModuleOp& module, std::ostream& os) {
        module->walk([&os](const mlir::func::FuncOp fn) {
            printMLIRFuncOp(fn, os);
        });
    }

} //namespace pyir
