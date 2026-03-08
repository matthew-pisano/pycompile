//
// Created by matthew on 3/6/26.
//

#include "pyir/pyir_codegen.h"

mlir::OwningOpRef<mlir::ModuleOp> generateMLIR(mlir::MLIRContext& ctx, const ByteCodeModule& module) {
    mlir::OpBuilder builder(&ctx);
    auto mlirModule = mlir::ModuleOp::create(builder.getUnknownLoc());
    builder.setInsertionPointToEnd(mlirModule.getBody());

    return mlirModule;
}
