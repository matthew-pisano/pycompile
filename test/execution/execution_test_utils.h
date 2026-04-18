//
// Created by matthew on 4/6/26.
//

#ifndef PYCOMPILE_EXECUTION_TEST_UTILS_H
#define PYCOMPILE_EXECUTION_TEST_UTILS_H

#include <iostream>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>

#include "conversion/pyir_codegen.h"
#include "lowering/llvm_export.h"
#include "lowering/pyir_lowering.h"
#include "pyir_run_module.h"
#include "pyruntime/builder_runtime.h"
#include "pyruntime/builtin_runtime.h"
#include "pyruntime/control_flow_runtime.h"
#include "pyruntime/function_runtime.h"
#include "pyruntime/logical_runtime.h"
#include "pyruntime/memory_runtime.h"


/**
 * Captures stdout output during JIT execution by redirecting the FILE* before
 * calling the JIT compiled function and restoring it afterward.
 *
 * @param fn The function to execute.
 * @return The output string from the function.
 */
static std::string captureStdout(const std::function<void()>& fn) {
    // Redirect stdout to a pipe
    int pipeFileDescriptor[2];
    pipe(pipeFileDescriptor);
    const int savedStdout = dup(STDOUT_FILENO);
    dup2(pipeFileDescriptor[1], STDOUT_FILENO);
    close(pipeFileDescriptor[1]);

    try {
        fn();
        fflush(stdout);
    } catch (...) {
        // Always restore stdout, even on exception
        dup2(savedStdout, STDOUT_FILENO);
        close(savedStdout);
        throw;
    }

    // Restore stdout
    dup2(savedStdout, STDOUT_FILENO);
    close(savedStdout);

    // Read captured output
    std::string output;
    char buf[256];
    ssize_t n;
    while ((n = read(pipeFileDescriptor[0], buf, sizeof(buf))) > 0)
        output.append(buf, n);
    close(pipeFileDescriptor[0]);

    return output;
}


/**
 * Runs a function with stdin redirected to read from the given string.
 *
 * @param fn The function to execute.
 * @param input The string to feed into stdin.
 */
static void withStdinInput(const std::function<void()>& fn, const std::string& input) {
    int pipeFileDescriptor[2];
    pipe(pipeFileDescriptor);

    write(pipeFileDescriptor[1], input.c_str(), input.size());
    close(pipeFileDescriptor[1]);

    const int savedStdin = dup(STDIN_FILENO);
    dup2(pipeFileDescriptor[0], STDIN_FILENO);
    close(pipeFileDescriptor[0]);

    try {
        fn();
    } catch (...) {
        // Always restore stdin, even on exception
        dup2(savedStdin, STDIN_FILENO);
        close(savedStdin);
        clearerr(stdin); // clear any EOF or error flags on the C stdin stream
        throw;
    }

    dup2(savedStdin, STDIN_FILENO);
    close(savedStdin);
    clearerr(stdin); // clear EOF flag set when pipe read end was exhausted
}


/**
 * Fixture that initializes the MLIR and LLVM contexts and compiles python strings to LLVM modules
 */
struct JITFixture {
    mlir::MLIRContext mlirCtx;
    llvm::LLVMContext llvmCtx;

    std::unique_ptr<llvm::Module> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const mlir::OwningOpRef<mlir::ModuleOp> mlirModule = generatePyIR(mlirCtx, bytecodeModule);
        lowerToLLVMDialect(mlirCtx, mlirModule);
        return translateToLLVMIR(llvmCtx, mlirModule);
    }

    /**
     * JIT compile and run the module, returning the exit code from main().
     */
    int run(const std::string& source) {
        std::unique_ptr<llvm::Module> llvmModule = compile(source);

        // Initialize native target (only needs to happen once, but is idempotent)
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        // Build the JIT
        auto jit = llvm::orc::LLJITBuilder().create();
        REQUIRE(jit);

        // Add the module
        auto tsm = llvm::orc::ThreadSafeModule(std::move(llvmModule), std::make_unique<llvm::LLVMContext>());
        llvm::Error err = (*jit)->addIRModule(std::move(tsm));
        REQUIRE(!err);

        // Expose runtime symbols so the JIT can resolve pyir_* calls
        llvm::orc::JITDylib& dylib = (*jit)->getMainJITDylib();
        llvm::orc::SymbolMap symbols;
        auto addSymbol = [&](const char* name, void* ptr) {
            const llvm::orc::ExecutorSymbolDef sym =
                    llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(ptr), llvm::JITSymbolFlags::Exported);
            symbols[(*jit)->getExecutionSession().intern(name)] = sym;
        };

        // Register all runtime functions
        addSymbol("pyir_initModule", reinterpret_cast<void*>(pyir_initModule));
        addSymbol("pyir_destroyModule", reinterpret_cast<void*>(pyir_destroyModule));
        addSymbol("pyir_loadName", reinterpret_cast<void*>(pyir_loadName));
        addSymbol("pyir_storeName", reinterpret_cast<void*>(pyir_storeName));
        addSymbol("pyir_loadFast", reinterpret_cast<void*>(pyir_loadFast));
        addSymbol("pyir_storeFast", reinterpret_cast<void*>(pyir_storeFast));
        addSymbol("pyir_pushScope", reinterpret_cast<void*>(pyir_pushScope));
        addSymbol("pyir_popScope", reinterpret_cast<void*>(pyir_popScope));
        addSymbol("pyir_makeFunction", reinterpret_cast<void*>(pyir_makeFunction));
        addSymbol("pyir_call", reinterpret_cast<void*>(pyir_call));
        addSymbol("pyir_loadConstInt", reinterpret_cast<void*>(pyir_loadConstInt));
        addSymbol("pyir_loadConstStr", reinterpret_cast<void*>(pyir_loadConstStr));
        addSymbol("pyir_loadConstFloat", reinterpret_cast<void*>(pyir_loadConstFloat));
        addSymbol("pyir_loadConstBool", reinterpret_cast<void*>(pyir_loadConstBool));
        addSymbol("pyir_loadConstNone", reinterpret_cast<void*>(pyir_loadConstNone));
        addSymbol("pyir_loadConstTuple", reinterpret_cast<void*>(pyir_loadConstTuple));
        addSymbol("pyir_loadAttr", reinterpret_cast<void*>(pyir_loadAttr));
        addSymbol("pyir_add", reinterpret_cast<void*>(pyir_add));
        addSymbol("pyir_sub", reinterpret_cast<void*>(pyir_sub));
        addSymbol("pyir_mul", reinterpret_cast<void*>(pyir_mul));
        addSymbol("pyir_pipe", reinterpret_cast<void*>(pyir_pipe));
        addSymbol("pyir_ampersand", reinterpret_cast<void*>(pyir_ampersand));
        addSymbol("pyir_idx", reinterpret_cast<void*>(pyir_idx));
        addSymbol("pyir_in", reinterpret_cast<void*>(pyir_in));
        addSymbol("pyir_div", reinterpret_cast<void*>(pyir_div));
        addSymbol("pyir_eq", reinterpret_cast<void*>(pyir_eq));
        addSymbol("pyir_ne", reinterpret_cast<void*>(pyir_ne));
        addSymbol("pyir_lt", reinterpret_cast<void*>(pyir_lt));
        addSymbol("pyir_le", reinterpret_cast<void*>(pyir_le));
        addSymbol("pyir_gt", reinterpret_cast<void*>(pyir_gt));
        addSymbol("pyir_ge", reinterpret_cast<void*>(pyir_ge));
        addSymbol("pyir_xor", reinterpret_cast<void*>(pyir_xor));
        addSymbol("pyir_isTruthy", reinterpret_cast<void*>(pyir_isTruthy));
        addSymbol("pyir_toBool", reinterpret_cast<void*>(pyir_toBool));
        addSymbol("pyir_unaryNot", reinterpret_cast<void*>(pyir_unaryNot));
        addSymbol("pyir_unaryNegative", reinterpret_cast<void*>(pyir_unaryNegative));
        addSymbol("pyir_unaryInvert", reinterpret_cast<void*>(pyir_unaryInvert));
        addSymbol("pyir_decref", reinterpret_cast<void*>(pyir_decref));
        addSymbol("pyir_formatSimple", reinterpret_cast<void*>(pyir_formatSimple));
        addSymbol("pyir_buildString", reinterpret_cast<void*>(pyir_buildString));
        addSymbol("pyir_buildList", reinterpret_cast<void*>(pyir_buildList));
        addSymbol("pyir_listExtend", reinterpret_cast<void*>(pyir_listExtend));
        addSymbol("pyir_listAppend", reinterpret_cast<void*>(pyir_listAppend));
        addSymbol("pyir_buildSet", reinterpret_cast<void*>(pyir_buildSet));
        addSymbol("pyir_setUpdate", reinterpret_cast<void*>(pyir_setUpdate));
        addSymbol("pyir_setAdd", reinterpret_cast<void*>(pyir_setAdd));
        addSymbol("pyir_buildMap", reinterpret_cast<void*>(pyir_buildMap));
        addSymbol("pyir_storeSubscr", reinterpret_cast<void*>(pyir_storeSubscr));
        addSymbol("pyir_builtinEnumerate", reinterpret_cast<void*>(pyir_builtinEnumerate));
        addSymbol("pyir_builtinIsInstance", reinterpret_cast<void*>(pyir_builtinIsInstance));
        addSymbol("pyir_builtinRange", reinterpret_cast<void*>(pyir_builtinRange));
        addSymbol("pyir_builtinType", reinterpret_cast<void*>(pyir_builtinType));
        addSymbol("pyir_builtinZip", reinterpret_cast<void*>(pyir_builtinZip));
        addSymbol("pyir_getIter", reinterpret_cast<void*>(pyir_getIter));
        addSymbol("pyir_forIter", reinterpret_cast<void*>(pyir_forIter));

        // Symbols for test error handling
        addSymbol("pyir_runModule", reinterpret_cast<void*>(pyir_runModule));
        addSymbol("pyir_getLastError", reinterpret_cast<void*>(pyir_getLastError));

        llvm::Error defineErr = dylib.define(llvm::orc::absoluteSymbols(symbols));
        REQUIRE(!defineErr);

        // Look up and call the module function directly, wrapping in try-catch
        llvm::Expected<llvm::orc::ExecutorAddr> moduleSym = (*jit)->lookup("__pymodule");
        REQUIRE(moduleSym);
        auto* moduleFn = moduleSym->toPtr<void()>();

        const int result = pyir_runModule(moduleFn);
        if (result != 0)
            throw std::runtime_error(pyir_getLastError());
        return result;
    }

    /**
     * Run and capture stdout output.
     */
    std::string runCapture(const std::string& source) {
        std::exception_ptr ex;
        std::string output = captureStdout([&] {
            try {
                run(source);
            } catch (...) {
                ex = std::current_exception();
            }
        });
        if (ex)
            std::rethrow_exception(ex);
        return output;
    }

    std::string runCaptureWithInput(const std::string& source, const std::string& input) {
        std::exception_ptr ex;
        std::string output = captureStdout([&] {
            withStdinInput(
                    [&] {
                        try {
                            run(source);
                        } catch (...) {
                            ex = std::current_exception();
                        }
                    },
                    input);
        });
        if (ex)
            std::rethrow_exception(ex);
        return output;
    }
};

#endif // PYCOMPILE_EXECUTION_TEST_UTILS_H
