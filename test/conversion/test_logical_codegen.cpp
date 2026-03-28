//
// Created by matthew on 3/25/26.
//

#include <catch2/catch_all.hpp>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>

#include "codegen_test_utils.h"
#include "pyir/pyir_ops.h"


TEST_CASE_METHOD(MLIRFixture, "Test Operation Order MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = 3\nd = (a + b) * c");
    const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    pyir::BinaryOp addOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 8));
    REQUIRE(addOp);
    REQUIRE(addOp.getOp() == "+");

    pyir::BinaryOp mulOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 10));
    REQUIRE(mulOp);
    REQUIRE(mulOp.getOp() == "*");
}


TEST_CASE_METHOD(MLIRFixture, "Test Arithmetic Operators MLIR") {
    SECTION("Test Addition") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a + b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "+");
    }

    SECTION("Test Float Addition") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2.1\nb = 2.1\nc = a + b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "+");
    }

    SECTION("Test List Addition") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = [1, 2]\nb = [3]\nc = a + b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 9));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "+");
    }

    SECTION("Test Subtraction") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a - b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "-");
    }

    SECTION("Test Multiplication") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a * b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "*");
    }
    SECTION("Test Division") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a / b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "/");
    }

    SECTION("Test Floor Division") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a // b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "//");
    }

    SECTION("Test Exponentiation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a ** b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "**");
    }

    SECTION("Test Modulo") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a % b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "%");
    }

    SECTION("Test Index") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = [1, 2]\nb = a[0]");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "[]");
    }

    SECTION("Test Integer Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = -a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryNegative unaryOp = mlir::dyn_cast<pyir::UnaryNegative>(getOp(fn, 3));
        REQUIRE(unaryOp);
    }

    SECTION("Test Binary Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = ~a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryInvert unaryOp = mlir::dyn_cast<pyir::UnaryInvert>(getOp(fn, 3));
        REQUIRE(unaryOp);
    }
}


TEST_CASE_METHOD(MLIRFixture, "Test Boolean Operators MLIR") {
    SECTION("Test Boolean Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = not a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryNot unaryOp = mlir::dyn_cast<pyir::UnaryNot>(getOp(fn, 4));
        REQUIRE(unaryOp);
    }

    SECTION("Test Boolean AND") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a and b");
        mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

        // Verify Op types
        REQUIRE(mlir::isa<pyir::IsTruthy>(getOp(fn, 6)));
        REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(getOp(fn, 7)));

        mlir::cf::CondBranchOp condBr = mlir::cast<mlir::cf::CondBranchOp>(getOp(fn, 7));
        REQUIRE(mlir::isa<pyir::IsTruthy>(condBr.getCondition().getDefiningOp()));

        // Verify blocks
        const llvm::iplist<mlir::Block>& blocks = fn.getBlocks();
        REQUIRE(blocks.size() == 3); // Entry block, true block, and false block

        mlir::Region::iterator blockIt = fn.getBody().begin();

        // Entry block should end with cond_br
        mlir::Block& entryBlock = *blockIt++;
        REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(entryBlock.getTerminator()));

        // True block
        mlir::Block& trueBlock = *blockIt++;
        REQUIRE(mlir::isa<mlir::func::ReturnOp>(trueBlock.getTerminator()));

        // False block
        mlir::Block& falseBlock = *blockIt++;
        REQUIRE(mlir::isa<mlir::cf::BranchOp>(falseBlock.getTerminator()));
    }

    SECTION("Test Boolean OR") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a or b");
        mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

        // Verify Op types
        REQUIRE(mlir::isa<pyir::IsTruthy>(getOp(fn, 6)));
        REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(getOp(fn, 7)));

        mlir::cf::CondBranchOp condBr = mlir::cast<mlir::cf::CondBranchOp>(getOp(fn, 7));
        REQUIRE(mlir::isa<pyir::IsTruthy>(condBr.getCondition().getDefiningOp()));

        // Verify blocks
        const llvm::iplist<mlir::Block>& blocks = fn.getBlocks();
        REQUIRE(blocks.size() == 3); // Entry block, true block, and false block

        mlir::Region::iterator blockIt = fn.getBody().begin();

        // Entry block should end with cond_br
        mlir::Block& entryBlock = *blockIt++;
        REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(entryBlock.getTerminator()));

        // True block
        mlir::Block& trueBlock = *blockIt++;
        REQUIRE(mlir::isa<mlir::func::ReturnOp>(trueBlock.getTerminator()));

        // False block
        mlir::Block& falseBlock = *blockIt++;
        REQUIRE(mlir::isa<mlir::cf::BranchOp>(falseBlock.getTerminator()));
    }

    SECTION("Test Boolean XOR") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a ^ b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "^");
    }

    SECTION("Test Bool()") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = bool(5)");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

        // Call op with valid arguments
        pyir::Call callOp = mlir::dyn_cast<pyir::Call>(getOp(fn, 3));
        REQUIRE(callOp);
        REQUIRE(callOp.getNumOperands() == 2);

        // The call op is for bool
        mlir::Operation* definingOp = callOp.getCallee().getDefiningOp();
        REQUIRE(mlir::isa<pyir::LoadName>(definingOp));
        pyir::LoadName loadName = mlir::cast<pyir::LoadName>(definingOp);
        REQUIRE(loadName.getVarName() == "bool");
    }
}


TEST_CASE_METHOD(MLIRFixture, "Test Comparators MLIR") {
    SECTION("Test Boolean Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = not a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryNot unaryOp = mlir::dyn_cast<pyir::UnaryNot>(getOp(fn, 4));
        REQUIRE(unaryOp);
    }

    SECTION("Test Boolean Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test Integer Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test Float Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2.1\nb = 2.1\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test String Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 'g'\nb = 'g'\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test Negative Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a != b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "!=");
    }

    SECTION("Test Less Than") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a < b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "<");
    }
    SECTION("Test Less Than Equal") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a <= b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "<=");
    }

    SECTION("Test Greater Than") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a > b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == ">");
    }

    SECTION("Test Greater Than Equal") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a >= b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 6));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == ">=");
    }
}
