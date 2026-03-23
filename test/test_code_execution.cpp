//
// Created by matthew on 3/22/26.
//

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>
#include <catch2/catch_all.hpp>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/OwningOpRef.h>

#include "bytecode/bytecode.h"
#include "lowering/llvm_export.h"
#include "lowering/pyir_to_llvm.h"
#include "pyir/pyir_codegen.h"
#include "pyir/pyir_runtime.h"


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

    fn();
    fflush(stdout);

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
 * Fixture that initializes the MLIR and LLVM contexts and compiles python strings to LLVM modules
 */
struct JITFixture {
    mlir::MLIRContext mlirCtx;
    llvm::LLVMContext llvmCtx;

    std::unique_ptr<llvm::Module> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const mlir::OwningOpRef<mlir::ModuleOp> mlirModule = pyir::generatePyIR(mlirCtx, bytecodeModule);
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
            const llvm::orc::ExecutorSymbolDef sym = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(ptr),
                llvm::JITSymbolFlags::Exported);
            symbols[(*jit)->getExecutionSession().intern(name)] = sym;
        };

        // Register all runtime functions
        addSymbol("pyir_load_name", reinterpret_cast<void*>(pyir_load_name));
        addSymbol("pyir_store_name", reinterpret_cast<void*>(pyir_store_name));
        addSymbol("pyir_load_fast", reinterpret_cast<void*>(pyir_load_fast));
        addSymbol("pyir_store_fast", reinterpret_cast<void*>(pyir_store_fast));
        addSymbol("pyir_push_scope", reinterpret_cast<void*>(pyir_push_scope));
        addSymbol("pyir_pop_scope", reinterpret_cast<void*>(pyir_pop_scope));
        addSymbol("pyir_make_function", reinterpret_cast<void*>(pyir_make_function));
        addSymbol("pyir_call", reinterpret_cast<void*>(pyir_call));
        addSymbol("pyir_load_const_int", reinterpret_cast<void*>(pyir_load_const_int));
        addSymbol("pyir_load_const_str", reinterpret_cast<void*>(pyir_load_const_str));
        addSymbol("pyir_load_const_float", reinterpret_cast<void*>(pyir_load_const_float));
        addSymbol("pyir_load_const_bool", reinterpret_cast<void*>(pyir_load_const_bool));
        addSymbol("pyir_load_const_none", reinterpret_cast<void*>(pyir_load_const_none));
        addSymbol("pyir_add", reinterpret_cast<void*>(pyir_add));
        addSymbol("pyir_sub", reinterpret_cast<void*>(pyir_sub));
        addSymbol("pyir_mul", reinterpret_cast<void*>(pyir_mul));
        addSymbol("pyir_div", reinterpret_cast<void*>(pyir_div));
        addSymbol("pyir_eq", reinterpret_cast<void*>(pyir_eq));
        addSymbol("pyir_ne", reinterpret_cast<void*>(pyir_ne));
        addSymbol("pyir_lt", reinterpret_cast<void*>(pyir_lt));
        addSymbol("pyir_le", reinterpret_cast<void*>(pyir_le));
        addSymbol("pyir_gt", reinterpret_cast<void*>(pyir_gt));
        addSymbol("pyir_ge", reinterpret_cast<void*>(pyir_ge));
        addSymbol("pyir_xor", reinterpret_cast<void*>(pyir_xor));
        addSymbol("pyir_is_truthy", reinterpret_cast<void*>(pyir_is_truthy));
        addSymbol("pyir_to_bool", reinterpret_cast<void*>(pyir_to_bool));
        addSymbol("pyir_unary_not", reinterpret_cast<void*>(pyir_unary_not));
        addSymbol("pyir_unary_negative", reinterpret_cast<void*>(pyir_unary_negative));
        addSymbol("pyir_unary_invert", reinterpret_cast<void*>(pyir_unary_invert));
        addSymbol("pyir_decref", reinterpret_cast<void*>(pyir_decref));
        addSymbol("pyir_format_simple", reinterpret_cast<void*>(pyir_format_simple));
        addSymbol("pyir_build_string", reinterpret_cast<void*>(pyir_build_string));

        llvm::Error defineErr = dylib.define(llvm::orc::absoluteSymbols(symbols));
        REQUIRE(!defineErr);

        // Look up and call main
        llvm::Expected<llvm::orc::ExecutorAddr> mainSym = (*jit)->lookup("main");
        REQUIRE(mainSym);
        auto* mainFn = mainSym->toPtr<int()>();
        return mainFn();
    }

    /**
     * Run and capture stdout output.
     */
    std::string runCapture(const std::string& source) {
        return captureStdout([&] { run(source); });
    }
};


TEST_CASE_METHOD(JITFixture, "JIT Hello World") {
    const std::string output = runCapture("print('Hello world!')");
    REQUIRE(output == "Hello world!\n");
}

TEST_CASE_METHOD(JITFixture, "JIT Function Call") {
    const std::string output = runCapture("def foo():\n  print('bar')\nfoo()");
    REQUIRE(output == "bar\n");
}

TEST_CASE_METHOD(JITFixture, "JIT Function With Arg") {
    const std::string output = runCapture("def foo(arg):\n  print(arg)\nfoo('bar')");
    REQUIRE(output == "bar\n");
}

TEST_CASE_METHOD(JITFixture, "JIT F-String") {
    const std::string output = runCapture("x = 42\nprint(f'Value is {x}')");
    REQUIRE(output == "Value is 42\n");
}
