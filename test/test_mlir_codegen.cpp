//
// Created by matthew on 3/8/26.
//

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
        return pyir::generatePyIR(ctx, bytecodeModule);
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
