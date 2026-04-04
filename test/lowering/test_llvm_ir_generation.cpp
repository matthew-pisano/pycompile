//
// Created by matthew on 3/22/26.
//


/**
 * Fixture that initializes the MLIR context and compiles python strings to MLIR modules
 */

#include <catch2/catch_all.hpp>
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>


#include "bytecode/bytecode.h"
#include "conversion/pyir_codegen.h"
#include "lowering/llvm_export.h"
#include "lowering/pyir_lowering.h"


/**
 * Fixture that initializes the MLIR and LLVM contexts and compiles python strings to LLVM modules
 */
struct LLVMFixture {
    mlir::MLIRContext mlirCtx;
    llvm::LLVMContext llvmCtx;

    std::unique_ptr<llvm::Module> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const mlir::OwningOpRef<mlir::ModuleOp> mlirModule = generatePyIR(mlirCtx, bytecodeModule);
        lowerToLLVMDialect(mlirCtx, mlirModule);
        std::unique_ptr<llvm::Module> llvmModule = translateToLLVMIR(llvmCtx, mlirModule);
        return llvmModule;
    }
};


TEST_CASE_METHOD(LLVMFixture, "Test F String Format LLVM") {
    const std::unique_ptr<llvm::Module> module = compile("f'Number <<{24}>>'");
}


TEST_CASE_METHOD(LLVMFixture, "Test Arithmetic Operators LLVM") {
    SECTION("Test Addition") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a + b"); }

    SECTION("Test Subtraction") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a - b"); }

    SECTION("Test Multiplication") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a * b"); }

    SECTION("Test Division") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a / b"); }

    SECTION("Test Floor Division") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a // b"); }

    SECTION("Test Exponentiation") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a ** b"); }

    SECTION("Test Modulo") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a % b"); }

    SECTION("Test Integer Negation") { const std::unique_ptr<llvm::Module> module = compile("a = True\nb = -a"); }

    SECTION("Test Binary Negation") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = ~a"); }
}

TEST_CASE_METHOD(LLVMFixture, "Test Boolean Operators LLVM") {
    SECTION("Test Boolean Negation") { const std::unique_ptr<llvm::Module> module = compile("a = True\nb = not a"); }

    SECTION("Test Boolean AND") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a and b");
    }

    SECTION("Test Boolean OR") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a or b");
    }

    SECTION("Test Boolean XOR") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a ^ b");
    }

    SECTION("Test Bool()") { const std::unique_ptr<llvm::Module> module = compile("a = bool(5)"); }
}


TEST_CASE_METHOD(LLVMFixture, "Test Comparators LLVM") {
    SECTION("Test Boolean Negation") { const std::unique_ptr<llvm::Module> module = compile("a = True\nb = not a"); }

    SECTION("Test Boolean Equality") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a == b");
    }

    SECTION("Test Integer Equality") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a == b");
    }

    SECTION("Test Float Equality") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2.1\nb = 2.1\nc = a == b");
    }

    SECTION("Test String Equality") {
        const std::unique_ptr<llvm::Module> module = compile("a = 'g'\nb = 'g'\nc = a == b");
    }

    SECTION("Test Negative Equality") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a != b");
    }

    SECTION("Test Less Than") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a < b"); }

    SECTION("Test Less Than Equal") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a <= b");
    }

    SECTION("Test Greater Than") { const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a > b"); }

    SECTION("Test Greater Than Equal") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a >= b");
    }
}


TEST_CASE_METHOD(LLVMFixture, "Test Conditional LLVM") {
    const std::unique_ptr<llvm::Module> module = compile("a = True\nif a:\n  ...\nelse:\n  ...");
}


TEST_CASE_METHOD(LLVMFixture, "Test Function Definition LLVM") {
    SECTION("Test Simple Function") {
        const std::unique_ptr<llvm::Module> module = compile("def foo():\n  print('bar')\nfoo()");
    }

    SECTION("Test Function with Args") {
        const std::unique_ptr<llvm::Module> module = compile("def foo(arg):\n  print(arg)\nfoo('bar')");
    }

    SECTION("Test Function with Return") {
        const std::unique_ptr<llvm::Module> module = compile("def foo():\n  return 'bar'\nprint(foo())");
    }
}


TEST_CASE_METHOD(LLVMFixture, "Test Container Operators LLVM") {
    SECTION("Test Index") { const std::unique_ptr<llvm::Module> module = compile("[1, 2][0]"); }

    SECTION("Test Membership") { const std::unique_ptr<llvm::Module> module = compile("1 in [1, 2]"); }
}


TEST_CASE_METHOD(LLVMFixture, "Test List Operations LLVM") {
    SECTION("Test Empty List") { const std::unique_ptr<llvm::Module> module = compile("a = list()"); }

    SECTION("Test Small List") { const std::unique_ptr<llvm::Module> module = compile("a = [1, 2, 3]"); }

    SECTION("Test Large List") {
        const std::unique_ptr<llvm::Module> module = compile(
                "a = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]");
    }

    SECTION("Test List Append") { const std::unique_ptr<llvm::Module> module = compile("[1, 2].append(3)"); }

    SECTION("Test List Extend") { const std::unique_ptr<llvm::Module> module = compile("[1, 2].extend([3])"); }
}


TEST_CASE_METHOD(LLVMFixture, "Test Set Operations LLVM") {
    SECTION("Test Empty Set") { const std::unique_ptr<llvm::Module> module = compile("a = set()"); }

    SECTION("Test Small Set") { const std::unique_ptr<llvm::Module> module = compile("a = {1, 2, 3}"); }

    SECTION("Test Large Set") {
        const std::unique_ptr<llvm::Module> module = compile(
                "a = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}");
    }

    SECTION("Test Set Add") { const std::unique_ptr<llvm::Module> module = compile("{1, 2}.add(3)"); }

    SECTION("Test Set Update") { const std::unique_ptr<llvm::Module> module = compile("{1, 2}.update({3})"); }
}


TEST_CASE_METHOD(LLVMFixture, "Test Hello World LLVM") {
    const std::unique_ptr<llvm::Module> module = compile("print('Hello world!')");
}
