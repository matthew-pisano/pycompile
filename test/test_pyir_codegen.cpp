//
// Created by matthew on 3/8/26.
//

#include <iostream>
#include <catch2/catch_all.hpp>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>


#include "pyir/pyir_codegen.h"
#include "pyir/pyir_attrs.h"
#include "bytecode/bytecode.h"


/**
 * Fixture that initializes the MLIR context and compiles python strings to MLIR modules
 */
struct MLIRFixture {
    mlir::MLIRContext mlirCtx;

    mlir::OwningOpRef<mlir::ModuleOp> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        mlir::OwningOpRef<mlir::ModuleOp> mlirModule = pyir::generatePyIR(mlirCtx, bytecodeModule);
        return mlirModule;
    }
};


/**
 * Gets the MLIR operation at the given index
 */
mlir::Operation* getOp(mlir::func::FuncOp fn, const int index) {
    auto it = fn.getBlocks().front().getOperations().begin();
    std::advance(it, index);
    return &*it;
}


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


TEST_CASE_METHOD(MLIRFixture, "Test F String Format MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("f'Number <<{24}>>'");
    const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Check first string component
    pyir::LoadConst loadConst = mlir::dyn_cast<pyir::LoadConst>(getOp(fn, 0));
    REQUIRE(loadConst);
    mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue());
    REQUIRE(strAttr);
    REQUIRE(strAttr.getValue() == "Number <<");

    // Check constant number
    pyir::LoadConst loadNumOp = mlir::dyn_cast<pyir::LoadConst>(getOp(fn, 1));
    REQUIRE(loadNumOp);
    mlir::IntegerAttr intAttr = mlir::dyn_cast<mlir::IntegerAttr>(loadNumOp.getValue());
    REQUIRE(intAttr);
    REQUIRE(intAttr.getInt() == 24);

    REQUIRE(mlir::isa<pyir::FormatSimple>(getOp(fn, 2)));

    // Check second string component
    loadConst = mlir::dyn_cast<pyir::LoadConst>(getOp(fn, 3));
    REQUIRE(loadConst);
    strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue());
    REQUIRE(strAttr);
    REQUIRE(strAttr.getValue() == ">>");

    REQUIRE(mlir::isa<pyir::BuildString>(getOp(fn, 4)));
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


TEST_CASE_METHOD(MLIRFixture, "Test Conditional MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nif a:\n  ...\nelse:\n  ...");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Verify Op types
    REQUIRE(mlir::isa<pyir::IsTruthy>(getOp(fn, 4)));
    REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(getOp(fn, 5)));

    mlir::cf::CondBranchOp condBr = mlir::cast<mlir::cf::CondBranchOp>(getOp(fn, 5));
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
    REQUIRE(mlir::isa<mlir::func::ReturnOp>(falseBlock.getTerminator()));
}


TEST_CASE_METHOD(MLIRFixture, "Test Function Definition MLIR") {
    SECTION("Simple Function") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("def foo():\n  print('bar')\nfoo()");

        // Collect all FuncOps in order: module fn, nested fn
        llvm::SmallVector<mlir::func::FuncOp> fns(
                (*module).getBody()->getOps<mlir::func::FuncOp>().begin(),
                (*module).getBody()->getOps<mlir::func::FuncOp>().end());

        mlir::func::FuncOp moduleFn = fns[0];
        mlir::func::FuncOp nestedFn = fns[1];

        // Validate module-level ops
        REQUIRE(mlir::isa<pyir::MakeFunction>(getOp(moduleFn, 0)));
        REQUIRE(mlir::isa<pyir::StoreName>(getOp(moduleFn, 1)));
        REQUIRE(mlir::isa<pyir::LoadName>(getOp(moduleFn, 2)));
        REQUIRE(mlir::isa<pyir::PushNull>(getOp(moduleFn, 3)));
        REQUIRE(mlir::isa<pyir::Call>(getOp(moduleFn, 4)));
        REQUIRE(mlir::isa<mlir::func::ReturnOp>(getOp(moduleFn, 5)));

        // make_function references the nested fn by name
        pyir::MakeFunction makeFunc = mlir::cast<pyir::MakeFunction>(getOp(moduleFn, 0));
        REQUIRE(makeFunc.getFnName().starts_with("__pyfn_"));

        // store_name stores it as "foo"
        pyir::StoreName storeName = mlir::cast<pyir::StoreName>(getOp(moduleFn, 1));
        REQUIRE(storeName.getVarName() == "foo");

        // call has no args (foo takes no arguments)
        pyir::Call callOp = mlir::cast<pyir::Call>(getOp(moduleFn, 4));
        REQUIRE(callOp.getNumOperands() == 1); // callee only, no args
        // Nested function __pyfn_*
        REQUIRE(nestedFn.getName().starts_with("__pyfn_"));
        // Signature: (!pyir.object, !pyir.object) -> !pyir.object
        REQUIRE(nestedFn.getFunctionType().getNumInputs() == 2);
        REQUIRE(nestedFn.getFunctionType().getNumResults() == 1);

        // Op sequence for function
        REQUIRE(mlir::isa<pyir::PushScope>(getOp(nestedFn, 0)));
        REQUIRE(mlir::isa<pyir::LoadName>(getOp(nestedFn, 1)));
        REQUIRE(mlir::isa<pyir::PushNull>(getOp(nestedFn, 2)));
        REQUIRE(mlir::isa<pyir::LoadConst>(getOp(nestedFn, 3)));
        REQUIRE(mlir::isa<pyir::Call>(getOp(nestedFn, 4)));
        REQUIRE(mlir::isa<pyir::LoadConst>(getOp(nestedFn, 5)));
        REQUIRE(mlir::isa<pyir::PopScope>(getOp(nestedFn, 6)));
        REQUIRE(mlir::isa<pyir::ReturnValue>(getOp(nestedFn, 7)));

        // load_name inside nested fn is "print"
        pyir::LoadName loadName = mlir::cast<pyir::LoadName>(getOp(nestedFn, 1));
        REQUIRE(loadName.getVarName() == "print");

        // load_const is "bar"
        pyir::LoadConst loadConst = mlir::cast<pyir::LoadConst>(getOp(nestedFn, 3));
        mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue());
        REQUIRE(strAttr);
        REQUIRE(strAttr.getValue() == "bar");

        // load_const(none) is a none attr
        pyir::LoadConst noneConst = mlir::cast<pyir::LoadConst>(getOp(nestedFn, 5));
        REQUIRE(mlir::isa<pyir::NoneAttr>(noneConst.getValue()));

        // return_value returns the none const
        pyir::ReturnValue retVal = mlir::cast<pyir::ReturnValue>(getOp(nestedFn, 7));
        REQUIRE(retVal.getValue().getDefiningOp() == noneConst.getOperation());
    }

    SECTION("Function with Args") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("def foo(arg):\n  print(arg)\nfoo('bar')");

        // Collect all FuncOps in order: module fn, nested fn
        llvm::SmallVector<mlir::func::FuncOp> fns(
                (*module).getBody()->getOps<mlir::func::FuncOp>().begin(),
                (*module).getBody()->getOps<mlir::func::FuncOp>().end());

        mlir::func::FuncOp moduleFn = fns[0];
        mlir::func::FuncOp nestedFn = fns[1];

        // Validate module-level ops
        REQUIRE(mlir::isa<pyir::MakeFunction>(getOp(moduleFn, 0)));
        REQUIRE(mlir::isa<pyir::StoreName>(getOp(moduleFn, 1)));
        REQUIRE(mlir::isa<pyir::LoadName>(getOp(moduleFn, 2)));
        REQUIRE(mlir::isa<pyir::PushNull>(getOp(moduleFn, 3)));
        REQUIRE(mlir::isa<pyir::LoadConst>(getOp(moduleFn, 4)));
        REQUIRE(mlir::isa<pyir::Call>(getOp(moduleFn, 5)));
        REQUIRE(mlir::isa<mlir::func::ReturnOp>(getOp(moduleFn, 6)));

        // make_function references the nested fn by name
        pyir::MakeFunction makeFunc = mlir::cast<pyir::MakeFunction>(getOp(moduleFn, 0));
        REQUIRE(makeFunc.getFnName().starts_with("__pyfn_"));

        // store_name stores it as "foo"
        pyir::StoreName storeName = mlir::cast<pyir::StoreName>(getOp(moduleFn, 1));
        REQUIRE(storeName.getVarName() == "foo");

        // call has one arg (foo takes one argument)
        pyir::Call callOp = mlir::cast<pyir::Call>(getOp(moduleFn, 5));
        REQUIRE(callOp.getNumOperands() == 2); // callee only, no args

        // Nested function __pyfn_*
        REQUIRE(nestedFn.getName().starts_with("__pyfn_"));
        // Signature: (!pyir.object, !pyir.object) -> !pyir.object
        REQUIRE(nestedFn.getFunctionType().getNumInputs() == 2);
        REQUIRE(nestedFn.getFunctionType().getNumResults() == 1);

        // Op sequence for function
        REQUIRE(mlir::isa<pyir::PushScope>(getOp(nestedFn, 0)));
        REQUIRE(mlir::isa<pyir::LoadArg>(getOp(nestedFn, 1)));
        REQUIRE(mlir::isa<pyir::StoreFast>(getOp(nestedFn, 2)));
        REQUIRE(mlir::isa<pyir::LoadName>(getOp(nestedFn, 3)));
        REQUIRE(mlir::isa<pyir::PushNull>(getOp(nestedFn, 4)));
        REQUIRE(mlir::isa<pyir::LoadFast>(getOp(nestedFn, 5)));
        REQUIRE(mlir::isa<pyir::Call>(getOp(nestedFn, 6)));
        REQUIRE(mlir::isa<pyir::LoadConst>(getOp(nestedFn, 7)));
        REQUIRE(mlir::isa<pyir::PopScope>(getOp(nestedFn, 8)));
        REQUIRE(mlir::isa<pyir::ReturnValue>(getOp(nestedFn, 9)));

        // load_name inside nested fn is "print"
        pyir::LoadName loadName = mlir::cast<pyir::LoadName>(getOp(nestedFn, 3));
        REQUIRE(loadName.getVarName() == "print");

        // load_fast is "arg"
        pyir::LoadFast loadFast = mlir::cast<pyir::LoadFast>(getOp(nestedFn, 5));
        REQUIRE(loadFast.getVarName() == "arg");

        // load_const(none) is a none attr
        pyir::LoadConst noneConst = mlir::cast<pyir::LoadConst>(getOp(nestedFn, 7));
        REQUIRE(mlir::isa<pyir::NoneAttr>(noneConst.getValue()));

        // return_value returns the none const
        pyir::ReturnValue retVal = mlir::cast<pyir::ReturnValue>(getOp(nestedFn, 9));
        REQUIRE(retVal.getValue().getDefiningOp() == noneConst.getOperation());
    }

    SECTION("Function with Return") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("def foo():\n  return 'bar'\nprint(foo())");

        // Collect all FuncOps in order: module fn, nested fn
        llvm::SmallVector<mlir::func::FuncOp> fns(
                (*module).getBody()->getOps<mlir::func::FuncOp>().begin(),
                (*module).getBody()->getOps<mlir::func::FuncOp>().end());

        mlir::func::FuncOp moduleFn = fns[0];
        mlir::func::FuncOp nestedFn = fns[1];

        // Validate module-level ops
        REQUIRE(mlir::isa<pyir::MakeFunction>(getOp(moduleFn, 0)));
        REQUIRE(mlir::isa<pyir::StoreName>(getOp(moduleFn, 1)));
        REQUIRE(mlir::isa<pyir::LoadName>(getOp(moduleFn, 2)));
        REQUIRE(mlir::isa<pyir::PushNull>(getOp(moduleFn, 3)));
        REQUIRE(mlir::isa<pyir::LoadName>(getOp(moduleFn, 4)));
        REQUIRE(mlir::isa<pyir::PushNull>(getOp(moduleFn, 5)));
        REQUIRE(mlir::isa<pyir::Call>(getOp(moduleFn, 6)));
        REQUIRE(mlir::isa<pyir::Call>(getOp(moduleFn, 7)));
        REQUIRE(mlir::isa<mlir::func::ReturnOp>(getOp(moduleFn, 8)));

        // make_function references the nested fn by name
        pyir::MakeFunction makeFunc = mlir::cast<pyir::MakeFunction>(getOp(moduleFn, 0));
        REQUIRE(makeFunc.getFnName().starts_with("__pyfn_"));

        // store_name stores it as "foo"
        pyir::StoreName storeName = mlir::cast<pyir::StoreName>(getOp(moduleFn, 1));
        REQUIRE(storeName.getVarName() == "foo");

        // call has no args (foo takes no arguments)
        pyir::Call callOp = mlir::cast<pyir::Call>(getOp(moduleFn, 6));
        REQUIRE(callOp.getNumOperands() == 1); // callee only, no args

        // load_name inside module is "print"
        pyir::LoadName loadName = mlir::cast<pyir::LoadName>(getOp(moduleFn, 2));
        REQUIRE(loadName.getVarName() == "print");

        // Nested function __pyfn_*
        REQUIRE(nestedFn.getName().starts_with("__pyfn_"));
        // Signature: (!pyir.object, !pyir.object) -> !pyir.object
        REQUIRE(nestedFn.getFunctionType().getNumInputs() == 2);
        REQUIRE(nestedFn.getFunctionType().getNumResults() == 1);

        // Op sequence for function
        REQUIRE(mlir::isa<pyir::PushScope>(getOp(nestedFn, 0)));
        REQUIRE(mlir::isa<pyir::LoadConst>(getOp(nestedFn, 1)));
        REQUIRE(mlir::isa<pyir::PopScope>(getOp(nestedFn, 2)));
        REQUIRE(mlir::isa<pyir::ReturnValue>(getOp(nestedFn, 3)));

        // load_const is "bar"
        pyir::LoadConst loadConst = mlir::cast<pyir::LoadConst>(getOp(nestedFn, 1));
        mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue());
        REQUIRE(strAttr);
        REQUIRE(strAttr.getValue() == "bar");

        // return_value returns the loaded const
        pyir::ReturnValue retVal = mlir::cast<pyir::ReturnValue>(getOp(nestedFn, 3));
        REQUIRE(retVal.getValue().getDefiningOp() == loadConst.getOperation());
    }
}


TEST_CASE_METHOD(MLIRFixture, "Test Hello World MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("print('Hello world!')");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Verify Op types
    REQUIRE(mlir::isa<pyir::LoadName>(getOp(fn, 0)));
    REQUIRE(mlir::isa<pyir::PushNull>(getOp(fn, 1)));
    REQUIRE(mlir::isa<pyir::LoadConst>(getOp(fn, 2)));
    REQUIRE(mlir::isa<pyir::Call>(getOp(fn, 3)));
    REQUIRE(mlir::isa<mlir::func::ReturnOp>(getOp(fn, 4)));

    // Call op with valid arguments
    pyir::Call callOp = mlir::dyn_cast<pyir::Call>(getOp(fn, 3));
    REQUIRE(callOp);
    REQUIRE(callOp.getNumOperands() == 2);

    // The call op is for print
    mlir::Operation* definingOp = callOp.getCallee().getDefiningOp();
    REQUIRE(mlir::isa<pyir::LoadName>(definingOp));
    pyir::LoadName loadName = mlir::cast<pyir::LoadName>(definingOp);
    REQUIRE(loadName.getVarName() == "print");

    // Print is being called on the correct string
    pyir::LoadConst loadConst = mlir::dyn_cast<pyir::LoadConst>(getOp(fn, 2));
    REQUIRE(loadConst);
    mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue());
    REQUIRE(strAttr);
    REQUIRE(strAttr.getValue() == "Hello world!");
}


TEST_CASE_METHOD(MLIRFixture, "Test Simple Arithmetic MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = 1\nb = 2\nsummed = a + b\nprint(summed)");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Verify Op types
    REQUIRE(mlir::isa<pyir::LoadConst>(getOp(fn, 0)));
    REQUIRE(mlir::isa<pyir::StoreName>(getOp(fn, 1)));
    REQUIRE(mlir::isa<pyir::LoadConst>(getOp(fn, 2)));
    REQUIRE(mlir::isa<pyir::StoreName>(getOp(fn, 3)));
    REQUIRE(mlir::isa<pyir::LoadName>(getOp(fn, 4)));
    REQUIRE(mlir::isa<pyir::LoadName>(getOp(fn, 5)));
    REQUIRE(mlir::isa<pyir::BinaryOp>(getOp(fn, 6)));
    REQUIRE(mlir::isa<pyir::StoreName>(getOp(fn, 7)));
    REQUIRE(mlir::isa<pyir::LoadName>(getOp(fn, 8)));
    REQUIRE(mlir::isa<pyir::PushNull>(getOp(fn, 9)));
    REQUIRE(mlir::isa<pyir::LoadName>(getOp(fn, 10)));
    REQUIRE(mlir::isa<pyir::Call>(getOp(fn, 11)));
    REQUIRE(mlir::isa<mlir::func::ReturnOp>(getOp(fn, 12)));

    // The integer 1 is being loaded
    pyir::LoadConst loadOneOp = mlir::dyn_cast<pyir::LoadConst>(getOp(fn, 0));
    REQUIRE(loadOneOp);
    mlir::IntegerAttr intAttr = mlir::dyn_cast<mlir::IntegerAttr>(loadOneOp.getValue());
    REQUIRE(intAttr);
    REQUIRE(intAttr.getInt() == 1);

    // The integer is being stored in variable a
    pyir::StoreName storeAOp = mlir::dyn_cast<pyir::StoreName>(getOp(fn, 1));
    REQUIRE(storeAOp);
    REQUIRE(storeAOp.getVarName() == "a");
    // The binary op is addition
    pyir::BinaryOp binaryOp = mlir::dyn_cast<pyir::BinaryOp>(getOp(fn, 6));
    REQUIRE(binaryOp);
    REQUIRE(binaryOp.getOp() == "+");

    // The binary op is being called on a and b
    mlir::Operation* lhsDef = binaryOp.getLhs().getDefiningOp();
    mlir::Operation* rhsDef = binaryOp.getRhs().getDefiningOp();
    REQUIRE(mlir::isa<pyir::LoadName>(lhsDef));
    REQUIRE(mlir::isa<pyir::LoadName>(rhsDef));
    pyir::LoadName lhsLoad = mlir::cast<pyir::LoadName>(lhsDef);
    pyir::LoadName rhsLoad = mlir::cast<pyir::LoadName>(rhsDef);
    REQUIRE(lhsLoad.getVarName() == "a");
    REQUIRE(rhsLoad.getVarName() == "b");

    // Call op with valid arguments
    pyir::Call callOp = mlir::dyn_cast<pyir::Call>(getOp(fn, 11));
    REQUIRE(callOp);
    REQUIRE(callOp.getNumOperands() == 2);

    // The call op is for print
    mlir::Operation* definingOp = callOp.getCallee().getDefiningOp();
    REQUIRE(mlir::isa<pyir::LoadName>(definingOp));
    pyir::LoadName loadName = mlir::cast<pyir::LoadName>(definingOp);
    REQUIRE(loadName.getVarName() == "print");
    // Print is being called on the correct variable
    REQUIRE(callOp.getArgs().size() == 1);
    pyir::LoadName argDef = mlir::dyn_cast<pyir::LoadName>(callOp.getArgs()[0].getDefiningOp());
    REQUIRE(argDef);
    REQUIRE(argDef.getVarName() == "summed");
}
