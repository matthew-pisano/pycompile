//
// Created by matthew on 3/24/26.
//


#include "pyir_conversion_utils.hpp"


mlir::LLVM::LLVMPointerType ptrType(mlir::MLIRContext* ctx) { return mlir::LLVM::LLVMPointerType::get(ctx); }


mlir::Type boolType(mlir::MLIRContext* ctx) { return mlir::IntegerType::get(ctx, 8); }


mlir::Type i64Type(mlir::MLIRContext* ctx) { return mlir::IntegerType::get(ctx, 64); }


mlir::Type i8Type(mlir::MLIRContext* ctx) { return mlir::IntegerType::get(ctx, 8); }


mlir::Type f64Type(mlir::MLIRContext* ctx) { return mlir::Float64Type::get(ctx); }


mlir::LLVM::LLVMFuncOp getOrInsertRuntimeFn(mlir::PatternRewriter& rewriter, mlir::ModuleOp module,
                                            const llvm::StringRef name, const mlir::LLVM::LLVMFunctionType fnType) {
    if (const mlir::LLVM::LLVMFuncOp fn = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>(name))
        return fn;

    mlir::PatternRewriter::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(module.getBody());
    return mlir::LLVM::LLVMFuncOp::create(rewriter, rewriter.getUnknownLoc(), name, fnType,
                                          mlir::LLVM::Linkage::External);
}


mlir::Value getOrInsertStringConstant(mlir::ConversionPatternRewriter& rewriter, mlir::ModuleOp module,
                                      const mlir::Location loc, const llvm::StringRef name, const llvm::StringRef str) {
    auto* ctx = module.getContext();

    if (!module.lookupSymbol(name)) {
        mlir::PatternRewriter::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(module.getBody());
        const auto strType = mlir::LLVM::LLVMArrayType::get(i8Type(ctx), str.size() + 1);
        mlir::LLVM::GlobalOp::create(rewriter, loc, strType, /*isConstant=*/true, mlir::LLVM::Linkage::Private, name,
                                     rewriter.getStringAttr(str.str() + '\0'));
    }

    const mlir::Value result = mlir::LLVM::AddressOfOp::create(rewriter, loc, ptrType(ctx), name);
    return result;
}


mlir::LogicalResult PyIROpConversion::insertRuntimeFunc(const std::string& func, mlir::Operation* op,
                                                        const mlir::ArrayRef<mlir::Value> operands,
                                                        mlir::ConversionPatternRewriter& rewriter,
                                                        const bool hasReturn) {
    mlir::MLIRContext* ctx = op->getContext();

    // Build argument type list: N copies of ptrType, one per operand
    const llvm::SmallVector<mlir::Type> argTypes(operands.size(), ptrType(ctx));
    const mlir::Type retType = hasReturn ? ptrType(ctx) : mlir::Type(mlir::LLVM::LLVMVoidType::get(ctx));
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(retType, argTypes);

    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, getModule(op), func, fnType);
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, op->getLoc(), fn, mlir::ValueRange(operands));

    if (hasReturn)
        rewriter.replaceOp(op, call.getResult());
    else
        rewriter.eraseOp(op);

    return mlir::success();
}
