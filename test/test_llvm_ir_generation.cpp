//
// Created by matthew on 3/22/26.
//


/**
 * Fixture that initializes the MLIR context and compiles python strings to MLIR modules
 */

#include <iostream>
#include <catch2/catch_all.hpp>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>


#include "pyir/pyir_codegen.h"
#include "pyir/pyir_attrs.h"
#include "bytecode/bytecode.h"
#include "lowering/llvm_export.h"


struct LLVMFixture {
    mlir::MLIRContext mlirCtx;
    llvm::LLVMContext llvmCtx;

    LLVMFixture() {
        mlirCtx.loadDialect<pyir::PyIRDialect>();
        mlirCtx.loadDialect<mlir::func::FuncDialect>();
        mlirCtx.loadDialect<mlir::cf::ControlFlowDialect>();
    }

    std::unique_ptr<llvm::Module> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const mlir::OwningOpRef<mlir::ModuleOp> mlirModule = pyir::generatePyIR(mlirCtx, bytecodeModule);
        std::unique_ptr<llvm::Module> llvmModule = translateToLLVMIR(llvmCtx, mlirModule);
        return llvmModule;
    }
};


TEST_CASE_METHOD(LLVMFixture, "Test Operation Order MLIR") {
    const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = 3\nd = (a + b) * c");
}


TEST_CASE_METHOD(LLVMFixture, "Test F String Format") {
    const std::unique_ptr<llvm::Module> module = compile("f'Number <<{24}>>'");
}


TEST_CASE_METHOD(LLVMFixture, "Test Arithmetic Operators MLIR") {
    SECTION("Test Addition") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a + b");
    }

    SECTION("Test Float Addition") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2.1\nb = 2.1\nc = a + b");
    }

    SECTION("Test Subtraction") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a - b");
    }

    SECTION("Test Multiplication") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a * b");
    }
    SECTION("Test Division") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a / b");
    }

    SECTION("Test Floor Division") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a // b");
    }

    SECTION("Test Exponentiation") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a ** b");
    }

    SECTION("Test Modulo") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a % b");
    }

    SECTION("Test Integer Negation") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = -a");
    }

    SECTION("Test Binary Negation") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = ~a");
    }
}


TEST_CASE_METHOD(LLVMFixture, "Test Boolean Operators MLIR") {
    SECTION("Test Boolean Negation") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = not a");
    }

    SECTION("Test Boolean AND") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a and b");
    }

    SECTION("Test Boolean OR") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a or b");
    }

    SECTION("Test Boolean XOR") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = True\nc = a ^ b");
    }

    SECTION("Test Bool()") {
        const std::unique_ptr<llvm::Module> module = compile("a = bool(5)");
    }
}


TEST_CASE_METHOD(LLVMFixture, "Test Comparators MLIR") {
    SECTION("Test Boolean Negation") {
        const std::unique_ptr<llvm::Module> module = compile("a = True\nb = not a");
    }

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

    SECTION("Test Less Than") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a < b");
    }
    SECTION("Test Less Than Equal") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a <= b");
    }

    SECTION("Test Greater Than") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a > b");
    }

    SECTION("Test Greater Than Equal") {
        const std::unique_ptr<llvm::Module> module = compile("a = 2\nb = 2\nc = a >= b");
    }
}


TEST_CASE_METHOD(LLVMFixture, "Test Conditional MLIR") {
    const std::unique_ptr<llvm::Module> module = compile("a = True\nif a:\n  ...\nelse:\n  ...");
}


TEST_CASE_METHOD(LLVMFixture, "Test Function Definition MLIR") {
    SECTION("Simple Function") {
        const std::unique_ptr<llvm::Module> module = compile("def foo():\n  print('bar')\nfoo()");
    }

    SECTION("Function with Args") {
        const std::unique_ptr<llvm::Module> module = compile("def foo(arg):\n  print(arg)\nfoo('bar')");
    }

    SECTION("Function with Return") {
        const std::unique_ptr<llvm::Module> module = compile("def foo():\n  return 'bar'\nprint(foo())");
    }
}


TEST_CASE_METHOD(LLVMFixture, "Test Hello World MLIR") {
    const std::unique_ptr<llvm::Module> module = compile("print('Hello world!')");
}


TEST_CASE_METHOD(LLVMFixture, "Test Simple Arithmetic MLIR") {
    const std::unique_ptr<llvm::Module> module = compile("a = 1\nb = 2\nsummed = a + b\nprint(summed)");
}
