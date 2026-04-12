//
// Created by matthew on 3/25/26.
//

#include <catch2/catch_all.hpp>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>

#include "codegen_test_utils.h"
#include "pyir/pyir_ops.h"


TEST_CASE_METHOD(MLIRFixture, "Test Conditional MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nif a:\n  ...\nelse:\n  ...");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Verify Op types
    REQUIRE(mlir::isa<pyir::IsTruthy>(getOp(fn, 5)));
    REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(getOp(fn, 6)));

    mlir::cf::CondBranchOp condBr = mlir::cast<mlir::cf::CondBranchOp>(getOp(fn, 6));
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


TEST_CASE_METHOD(MLIRFixture, "Test While MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("a = True\nwhile a:\n  ...");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Verify Op types
    REQUIRE(mlir::isa<mlir::cf::BranchOp>(getOp(fn, 3)));

    // Verify blocks
    const llvm::iplist<mlir::Block>& blocks = fn.getBlocks();
    REQUIRE(blocks.size() == 4); // Entry block, true block, and false block

    mlir::Region::iterator blockIt = fn.getBody().begin();

    // Entry block should end with br
    mlir::Block& entryBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::cf::BranchOp>(entryBlock.getTerminator()));

    // Post loop block
    mlir::Block& endBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::func::ReturnOp>(endBlock.getTerminator()));

    // Loop Condition block
    mlir::Block& conditionBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(conditionBlock.getTerminator()));

    // Loop body block
    mlir::Block& bodyBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::cf::BranchOp>(bodyBlock.getTerminator()));
}


TEST_CASE_METHOD(MLIRFixture, "Test For MLIR") {
    const mlir::OwningOpRef<mlir::ModuleOp> module = compile("for i in range(5):\n  ...");
    mlir::func::FuncOp fn = *(*module).getBody()->getOps<mlir::func::FuncOp>().begin();

    // Verify Op types
    REQUIRE(mlir::isa<mlir::cf::BranchOp>(getOp(fn, 3)));

    // Verify blocks
    const llvm::iplist<mlir::Block>& blocks = fn.getBlocks();
    REQUIRE(blocks.size() == 4); // Entry block, true block, and false block

    mlir::Region::iterator blockIt = fn.getBody().begin();

    // Entry block should end with br
    mlir::Block& entryBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::cf::BranchOp>(entryBlock.getTerminator()));

    // Post loop block
    mlir::Block& endBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::func::ReturnOp>(endBlock.getTerminator()));

    // Loop Condition block
    mlir::Block& conditionBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::cf::CondBranchOp>(conditionBlock.getTerminator()));

    // Loop body block
    mlir::Block& bodyBlock = *blockIt++;
    REQUIRE(mlir::isa<mlir::cf::BranchOp>(bodyBlock.getTerminator()));
}