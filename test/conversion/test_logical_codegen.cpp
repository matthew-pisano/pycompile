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

    pyir::BinaryOp addOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 9));
    REQUIRE(addOp);
    REQUIRE(addOp.getOp() == "+");

    pyir::BinaryOp mulOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 11));
    REQUIRE(mulOp);
    REQUIRE(mulOp.getOp() == "*");
}


TEST_CASE_METHOD(MLIRFixture, "Test Arithmetic Operators MLIR") {
    SECTION("Test Addition") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a + b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "+");
    }

    SECTION("Test Float Addition") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2.1\nb = 2.1\nc = a + b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "+");
    }

    SECTION("Test Subtraction") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a - b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "-");
    }

    SECTION("Test Multiplication") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a * b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "*");
    }

    SECTION("Test Division") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a / b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "/");
    }

    SECTION("Test Floor Division") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a // b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "//");
    }

    SECTION("Test Exponentiation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a ** b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "**");
    }

    SECTION("Test Modulo") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a % b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "%");
    }

    SECTION("Test Integer Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = -a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryNegative unaryOp = mlir::dyn_cast<pyir::UnaryNegative>(getOp(fn, 4));
        REQUIRE(unaryOp);
    }

    SECTION("Test Binary Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = ~a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryInvert unaryOp = mlir::dyn_cast<pyir::UnaryInvert>(getOp(fn, 4));
        REQUIRE(unaryOp);
    }
}


TEST_CASE_METHOD(MLIRFixture, "Test Boolean Operators MLIR") {
    SECTION("Test Boolean Negation") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = not a");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::UnaryNot unaryOp = mlir::dyn_cast<pyir::UnaryNot>(getOp(fn, 5));
        REQUIRE(unaryOp);
    }

    SECTION("Test Boolean AND") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a and b");
        mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

        // Verify Op types
        REQUIRE(mlir::isa<pyir::IsTruthy>(getOp(fn, 7)));
        REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(getOp(fn, 8)));

        mlir::cf::CondBranchOp condBr = mlir::cast<mlir::cf::CondBranchOp>(getOp(fn, 8));
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
        REQUIRE(mlir::isa<pyir::IsTruthy>(getOp(fn, 7)));
        REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(getOp(fn, 8)));

        mlir::cf::CondBranchOp condBr = mlir::cast<mlir::cf::CondBranchOp>(getOp(fn, 8));
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
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "^");
    }

    SECTION("Test Bool()") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = bool(5)");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

        // Call op with valid arguments
        pyir::Call callOp = mlir::dyn_cast<pyir::Call>(getOp(fn, 4));
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
        pyir::UnaryNot unaryOp = mlir::dyn_cast<pyir::UnaryNot>(getOp(fn, 5));
        REQUIRE(unaryOp);
    }

    SECTION("Test Boolean Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test Integer Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test Float Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2.1\nb = 2.1\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test String Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 'g'\nb = 'g'\nc = a == b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "==");
    }

    SECTION("Test Negative Equality") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nb = True\nc = a != b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "!=");
    }

    SECTION("Test Less Than") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a < b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "<");
    }
    SECTION("Test Less Than Equal") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a <= b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "<=");
    }

    SECTION("Test Greater Than") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a > b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == ">");
    }

    SECTION("Test Greater Than Equal") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 2\nb = 2\nc = a >= b");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::CompareOp binaryOp = mlir::dyn_cast<pyir::CompareOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == ">=");
    }
}


TEST_CASE_METHOD(MLIRFixture, "Test Container Operators MLIR") {
    SECTION("Test Index") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = [1, 2]\nb = a[0]");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 7));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "[]");
    }

    SECTION("Test Membership") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("1 in [1, 2]");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::ContainsOp containsOp = mlir::dyn_cast<pyir::ContainsOp>(getOp(fn, 3));
        REQUIRE(containsOp);
        REQUIRE(containsOp.getOp() == "in");
    }

    SECTION("Test Union") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("{1} | {2}");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "|");
    }

    SECTION("Test Intersection") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("{1} & {2}");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "&");
    }
}


TEST_CASE_METHOD(MLIRFixture, "Test Arithmetic Assignment Operators MLIR") {
    SECTION("Test Addition Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na += 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "+=");
    }

    SECTION("Test Subtraction Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na -= 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "-=");
    }

    SECTION("Test Multiplication Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na *= 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "*=");
    }

    SECTION("Test Division Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na /= 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "/=");
    }

    SECTION("Test Floor Division Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na //= 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "//=");
    }

    SECTION("Test Exponentiation Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na **= 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "**=");
    }

    SECTION("Test Modulo Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\na %= 1");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "%=");
    }
}

TEST_CASE_METHOD(MLIRFixture, "Test Boolean Assignment Operators MLIR") {
    SECTION("Test AND Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\na &= False");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "&=");
    }

    SECTION("Test OR Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\na |= False");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "|=");
    }

    SECTION("Test XOR Assignment") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\na ^= False");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
        pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 5));
        REQUIRE(binaryOp);
        REQUIRE(binaryOp.getOp() == "^=");
    }
}
