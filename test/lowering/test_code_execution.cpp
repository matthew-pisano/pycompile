//
// Created by matthew on 3/22/26.
//

#include <catch2/catch_all.hpp>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/OwningOpRef.h>

#include "bytecode/bytecode.h"
#include "conversion/pyir_codegen.h"
#include "lowering/llvm_export.h"
#include "lowering/pyir_lowering.h"
#include "pyruntime/builder_runtime.h"
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


TEST_CASE_METHOD(JITFixture, "Test JIT Hello World") {
    const std::string output = runCapture("print('Hello world!')");
    REQUIRE(output == "Hello world!\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Function Call") {
    const std::string output = runCapture("def foo():\n  print('bar')\nfoo()");
    REQUIRE(output == "bar\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Function With Arg") {
    const std::string output = runCapture("def foo(arg):\n  print(arg)\nfoo('bar')");
    REQUIRE(output == "bar\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT String Operations") {

    SECTION("Test F-String") {
        const std::string output = runCapture("x = 42\nprint(f'Value is {x}')");
        REQUIRE(output == "Value is 42\n");
    }

    SECTION("Test List Index") {
        const std::string output = runCapture("a = 'Hello there'\nprint(a[2])");
        REQUIRE(output == "l\n");
    }

    SECTION("Test Set as String") {
        const std::string output =
                runCapture("a = str({1, 2, 3})\nprint(len(a) == 9 and ('1' in a) and ('2' in a) and ('3' in a))");
        REQUIRE(output == "True\n");
    }

    SECTION("Test List as String") {
        const std::string output = runCapture("print(str([1, 2, 3]))");
        REQUIRE(output == "[1, 2, 3]\n");
    }
}


TEST_CASE_METHOD(JITFixture, "Test JIT List Operations") {
    SECTION("Test Build List") {
        std::string output = runCapture("print([])");
        REQUIRE(output == "[]\n");
        output = runCapture("print(list())");
        REQUIRE(output == "[]\n");
    }

    SECTION("Test List Extend Op") {
        const std::string output = runCapture("print([1, 2, 3])");
        REQUIRE(output == "[1, 2, 3]\n");
    }

    SECTION("Test List Append Op") {
        std::string listStr =
                "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]";
        const std::string output = runCapture(std::format("print({})", listStr));
        REQUIRE(output == listStr + "\n");
    }

    SECTION("Test List Equality") {
        std::string output = runCapture("print([1, 2, 3] == [1, 2, 3])");
        REQUIRE(output == "True\n");

        output = runCapture("print([1, 3, 2] == [1, 2, 3])");
        REQUIRE(output == "False\n");

        output = runCapture("print([1, 2, 3] == [3])");
        REQUIRE(output == "False\n");
    }

    SECTION("Test List Addition") {
        const std::string output = runCapture("print([1, 2] + [3])");
        REQUIRE(output == "[1, 2, 3]\n");
    }

    SECTION("Test List Index") {
        const std::string output = runCapture("a = [1, 2]\nprint(a[0])");
        REQUIRE(output == "1\n");
    }

    SECTION("Test List Length") {
        const std::string output = runCapture("print(len([1, 2]))");
        REQUIRE(output == "2\n");
    }

    SECTION("Test List Membership") {
        const std::string output = runCapture("a = [1, 2]\nprint(1 in a)");
        REQUIRE(output == "True\n");
    }

    SECTION("Test List Append") {
        const std::string output = runCapture("a = [1, 2]\na.append(3)\nprint(a)");
        REQUIRE(output == "[1, 2, 3]\n");
    }

    SECTION("Test List Extend") {
        const std::string output = runCapture("a = [1, 2]\na.extend([3])\nprint(a)");
        REQUIRE(output == "[1, 2, 3]\n");
    }

    SECTION("Test String as List") {
        const std::string output = runCapture("print(list('Hello'))");
        REQUIRE(output == "['H', 'e', 'l', 'l', 'o']\n");
    }

    SECTION("Test Set as List") {
        const std::string output =
                runCapture("a = list({1, 2, 3})\nprint(len(a) == 3 and (1 in a) and (2 in a) and (3 in a))");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Tuple as List") {
        const std::string output = runCapture("print(list((1, 2, 3)))");
        REQUIRE(output == "[1, 2, 3]\n");
    }
}


TEST_CASE_METHOD(JITFixture, "Test JIT Set Operations") {
    SECTION("Test Build Set") {
        const std::string output = runCapture("print(set())");
        REQUIRE(output == "set()\n");
    }

    SECTION("Test Set Update Op") {
        const std::string output = runCapture("print({1, 1, 1})");
        REQUIRE(output == "{1}\n");
    }

    SECTION("Test Set Add Op") {
        const std::string output = runCapture(
                "print({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})");
        REQUIRE(output == "{1}\n");
    }

    SECTION("Test Set Equality") {
        std::string output = runCapture("print({1, 2, 3} == {1, 2, 3})");
        REQUIRE(output == "True\n");

        output = runCapture("print({1, 3, 2} == {1, 2, 3})");
        REQUIRE(output == "True\n");

        output = runCapture("print({1, 2, 3} == {3})");
        REQUIRE(output == "False\n");
    }

    SECTION("Test Set Union") {
        const std::string output = runCapture("print({1, 2} | {3} == {1, 2, 3})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Set Intersection") {
        const std::string output = runCapture("print({1, 2} & {2, 3} == {2})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Set Difference") {
        const std::string output = runCapture("print({1, 2, 3} - {1, 4} == {2, 3})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Set Symmetric Difference") {
        const std::string output = runCapture("print({1, 2} ^ {2, 3} == {1, 3})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Set Membership") {
        const std::string output = runCapture("a = {1, 2}\nprint(1 in a)");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Set Length") {
        const std::string output = runCapture("print(len({1, 2}))");
        REQUIRE(output == "2\n");
    }

    SECTION("Test Set Add") {
        const std::string output = runCapture("a = {1, 2}\na.add(3)\nprint(a == {1, 2, 3})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Set Update") {
        const std::string output = runCapture("a = {1, 2}\na.update({3})\nprint(a == {1, 2, 3})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test String as Set") {
        const std::string output = runCapture("print(set('Hello') == {'H', 'e', 'l', 'o'})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test List as Set") {
        const std::string output = runCapture("a = set([1, 2, 3])\nprint(a == {1, 2, 3})");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Tuple as Set") {
        const std::string output = runCapture("a = set((1, 2, 3))\nprint(a == {1, 2, 3})");
        REQUIRE(output == "True\n");
    }
}


TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Operations") {
    SECTION("Test Load Tuple") {
        const std::string output = runCapture("print(tuple())");
        REQUIRE(output == "()\n");
    }

    SECTION("Test Single Element Tuple") {
        const std::string output = runCapture("print((2,))");
        REQUIRE(output == "(2,)\n");
    }

    SECTION("Test Multi Element Tuple") {
        const std::string output = runCapture("print((1, 2, 3))");
        REQUIRE(output == "(1, 2, 3)\n");
    }

    SECTION("Test Tuple Equality") {
        std::string output = runCapture("print((1, 2, 3) == (1, 2, 3))");
        REQUIRE(output == "True\n");

        output = runCapture("print((1, 3, 2) == (1, 2, 3))");
        REQUIRE(output == "False\n");

        output = runCapture("print((1, 2, 3) == (3,))");
        REQUIRE(output == "False\n");
    }

    SECTION("Test Tuple Add") {
        const std::string output = runCapture("print((1, 2, 3) + (4,))");
        REQUIRE(output == "(1, 2, 3, 4)\n");
    }

    SECTION("Test Tuple Membership") {
        const std::string output = runCapture("a = (1, 2)\nprint(1 in a)");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Tuple Index") {
        const std::string output = runCapture("a = (1, 2)\nprint(a[0])");
        REQUIRE(output == "1\n");
    }

    SECTION("Test Tuple Length") {
        const std::string output = runCapture("print(len((1, 2)))");
        REQUIRE(output == "2\n");
    }

    SECTION("Test String as Tuple") {
        const std::string output = runCapture("print(tuple('Hello'))");
        REQUIRE(output == "('H', 'e', 'l', 'l', 'o')\n");
    }

    SECTION("Test List as Tuple") {
        const std::string output = runCapture("print(tuple([1, 2, 3]))");
        REQUIRE(output == "(1, 2, 3)\n");
    }

    SECTION("Test Set as Tuple") {
        const std::string output =
                runCapture("a = tuple({1, 2, 3})\nprint(len(a) == 3 and (1 in a) and (2 in a) and (3 in a))");
        REQUIRE(output == "True\n");
    }
}


TEST_CASE_METHOD(JITFixture, "Test JIT Dict Operations") {
    SECTION("Test Build Dict") {
        std::string output = runCapture("print({})");
        REQUIRE(output == "{}\n");

        output = runCapture("print(dict())");
        REQUIRE(output == "{}\n");

        output = runCapture("print({1: 'one', 2: 'two', 3: 'three'})");
        REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
    }

    SECTION("Test Dict Equality") {
        std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'} == {1: 'one', 2: 'two', 3: 'three'})");
        REQUIRE(output == "True\n");

        output = runCapture("print({1: 'one', 2: 'two', 3: 'three'} == {1: 'one', 3: 'three', 2: 'two'})");
        REQUIRE(output == "True\n");

        output = runCapture("print({1: 'one', 2: 'two', 3: 'three'} == {1: 'one', 2: 'two'})");
        REQUIRE(output == "False\n");
    }

    SECTION("Test Dict Merge") {
        const std::string output = runCapture("print({2: 'two', 3: 'three'} | {1: 'one'})");
        REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
    }

    SECTION("Test Dict Membership") {
        const std::string output = runCapture("a = {1: 'one', 2: 'two', 3: 'three'}\nprint(1 in a)");
        REQUIRE(output == "True\n");
    }

    SECTION("Test Dict Length") {
        const std::string output = runCapture("print(len({1: 'one', 2: 'two', 3: 'three'}))");
        REQUIRE(output == "3\n");
    }

    SECTION("Test Dict Get") {
        std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.get(2))");
        REQUIRE(output == "two\n");

        output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.get('z'))");
        REQUIRE(output == "None\n");
    }

    SECTION("Test Dict Keys") {
        const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.keys())");
        REQUIRE(output == "(1, 2, 3)\n");
    }

    SECTION("Test Dict Values") {
        const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.values())");
        REQUIRE(output == "('one', 'two', 'three')\n");
    }

    SECTION("Test Dict Items") {
        const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.items())");
        REQUIRE(output == "[(1, 'one'), (2, 'two'), (3, 'three')]\n");
    }

    SECTION("Test Dict Update") {
        const std::string output = runCapture("a = {2: 'two', 3: 'three'}\na.update({1: 'one'})\nprint(a)");
        REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
    }
}
