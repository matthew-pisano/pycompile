//
// Created by matthew on 3/25/26.
//

#include <catch2/catch_all.hpp>

#include "codegen_test_utils.h"
#include "pyir/pyir_attrs.h"
#include "pyir/pyir_ops.h"


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


TEST_CASE_METHOD(MLIRFixture, "Test Build List MLIR") {

    SECTION("Test Empty List") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = []");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
    }

    SECTION("Test Small List") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = [1, 2, 3]");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
    }

    SECTION("Test Large List") {
        const mlir::OwningOpRef<mlir::ModuleOp> module = compile(
                "a = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]");
        const mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();
    }
}
