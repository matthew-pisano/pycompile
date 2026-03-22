//
// Created by matthew on 3/8/26.
//

#include <iostream>
#include <catch2/catch_all.hpp>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>


#include "pyir/pyir_codegen.h"
#include "bytecode/bytecode.h"


/**
 * Fixture that initializes the MLIR context and compiles python strings to MLIR modules
 */
struct MLIRFixture {
    mlir::MLIRContext ctx;

    MLIRFixture() {
        ctx.loadDialect<pyir::PyIRDialect>();
        ctx.loadDialect<mlir::func::FuncDialect>();
        ctx.loadDialect<mlir::cf::ControlFlowDialect>();
    }

    mlir::OwningOpRef<mlir::ModuleOp> compile(const std::string& source) {
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");

        std::stringstream ss;
        serializeByteCodeModule(bytecodeModule, ss);
        std::cout << ss.str() << std::endl;

        mlir::OwningOpRef<mlir::ModuleOp> module = pyir::generatePyIR(ctx, bytecodeModule);

        std::string mlirModuleContent;
        llvm::raw_string_ostream llvmOs(mlirModuleContent);
        const mlir::OpPrintingFlags flags;
        module.get().getOperation()->print(llvmOs, flags);
        std::cout << mlirModuleContent << std::endl;

        return module;
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


TEST_CASE_METHOD(MLIRFixture, "Test F String Format") {
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
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
    }

    SECTION("Function with Args") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("def foo(arg):\n  print(arg)\nfoo('bar')");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
    }

    SECTION("Function with Return") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("def foo():\n  return 'bar'\nprint(foo())");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
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
