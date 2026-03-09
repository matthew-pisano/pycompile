//
// Created by matthew on 3/8/26.
//

#include <catch2/catch_all.hpp>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>


#include "pyir/pyir_codegen.h"
#include "bytecode/bytecode.h"
#include "bytecode/pythoncode.h"


struct MLIRFixture {
    mlir::MLIRContext ctx;

    MLIRFixture() {
        ctx.loadDialect<pyir::PyIRDialect>();
        ctx.loadDialect<mlir::func::FuncDialect>();
        ctx.loadDialect<mlir::cf::ControlFlowDialect>();
    }

    mlir::OwningOpRef<mlir::ModuleOp> compile(const std::string& source) {
        // compile source through your pipeline
        const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
        const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
        return pyir::generateMLIR(ctx, bytecodeModule);
    }
};


mlir::Operation* getOp(mlir::func::FuncOp fn, const int index) {
    auto it = fn.getBlocks().front().getOperations().begin();
    std::advance(it, index);
    return &*it;
}


TEST_CASE_METHOD(MLIRFixture, "Test Hello World") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("print('Hello world!')");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    REQUIRE(mlir::isa<pyir::LoadName>(getOp(fn, 0)));
    REQUIRE(mlir::isa<pyir::PushNull>(getOp(fn, 1)));
    REQUIRE(mlir::isa<pyir::LoadConst>(getOp(fn, 2)));
    REQUIRE(mlir::isa<pyir::Call>(getOp(fn, 3)));
    REQUIRE(mlir::isa<mlir::func::ReturnOp>(getOp(fn, 4)));

    pyir::Call callOp = mlir::dyn_cast<pyir::Call>(getOp(fn, 3));
    REQUIRE(callOp);
    REQUIRE(callOp.getNumOperands() == 2); // callee + 1 arg

    // verify callee is a load_name
    mlir::Operation* definingOp = callOp.getCallee().getDefiningOp();
    REQUIRE(mlir::isa<pyir::LoadName>(definingOp));

    // verify the name
    pyir::LoadName loadName = mlir::cast<pyir::LoadName>(definingOp);
    REQUIRE(loadName.getVarName() == "print");

    pyir::LoadConst loadConst = mlir::dyn_cast<pyir::LoadConst>(getOp(fn, 2));
    REQUIRE(loadConst);

    auto strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue());
    REQUIRE(strAttr);
    REQUIRE(strAttr.getValue() == "Hello world!");
}
