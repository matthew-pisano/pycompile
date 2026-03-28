//
// Created by matthew on 3/25/26.
//

#include <catch2/catch_all.hpp>

#include "codegen_test_utils.h"
#include "pyir/pyir_attrs.h"
#include "pyir/pyir_ops.h"


TEST_CASE_METHOD(MLIRFixture, "Test Function Definition MLIR") {
    SECTION("Simple Function") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("def foo():\n  print('bar')\nfoo()");

        // Collect all FuncOps in order: module fn, nested fn
        llvm::SmallVector<mlir::func::FuncOp> fns((*module).getBody()->getOps<mlir::func::FuncOp>().begin(),
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
        llvm::SmallVector<mlir::func::FuncOp> fns((*module).getBody()->getOps<mlir::func::FuncOp>().begin(),
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
        llvm::SmallVector<mlir::func::FuncOp> fns((*module).getBody()->getOps<mlir::func::FuncOp>().begin(),
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


TEST_CASE_METHOD(MLIRFixture, "Test Bound Method MLIR") {
    SECTION("Test List Append") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("[1, 2, 3].append(4)");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
    }
}
