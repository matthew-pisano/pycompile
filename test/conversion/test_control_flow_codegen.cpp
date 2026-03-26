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
