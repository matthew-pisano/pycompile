//
// Created by matthew on 3/24/26.
//

#include "lowering/logical_lowering.h"


mlir::LogicalResult IsTruthyLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern int8_t pyir_isTruthy(Value* val)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::IntegerType::get(ctx, 8), {ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_isTruthy", fnType);

    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0]});

    // truncate i8 to i1 for cf.cond_br
    const mlir::Value i1val =
            rewriter.create<mlir::LLVM::TruncOp>(loc, mlir::IntegerType::get(ctx, 1), call.getResult());

    rewriter.replaceOp(op, i1val);
    return mlir::success();
}


mlir::LogicalResult ToBoolLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                    mlir::ConversionPatternRewriter& rewriter) const {
    return linkOpToRuntimeFunc("pyir_toBool", op, operands, rewriter, 1);
}


mlir::LogicalResult UnaryInvertLowering::matchAndRewrite(mlir::Operation* op,
                                                         const mlir::ArrayRef<mlir::Value> operands,
                                                         mlir::ConversionPatternRewriter& rewriter) const {
    return linkOpToRuntimeFunc("pyir_unaryInvert", op, operands, rewriter, 1);
}


mlir::LogicalResult UnaryNegativeLowering::matchAndRewrite(mlir::Operation* op,
                                                           const mlir::ArrayRef<mlir::Value> operands,
                                                           mlir::ConversionPatternRewriter& rewriter) const {
    return linkOpToRuntimeFunc("pyir_unaryNegative", op, operands, rewriter, 1);
}


mlir::LogicalResult UnaryNotLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    return linkOpToRuntimeFunc("pyir_unaryNot", op, operands, rewriter, 1);
}


mlir::LogicalResult BinaryOpLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    pyir::BinaryOp binaryOp = mlir::cast<pyir::BinaryOp>(op);

    // Map operator string to runtime function name
    static const std::unordered_map<std::string, std::string> opToFn = {
            {"+", "pyir_add"}, {"-", "pyir_sub"},       {"*", "pyir_mul"},  {"/", "pyir_div"},
            {"^", "pyir_xor"}, {"//", "pyir_floorDiv"}, {"**", "pyir_exp"}, {"%", "pyir_mod"},
    };

    const std::string opStr = binaryOp.getOp().str();
    const auto it = opToFn.find(opStr);
    if (it == opToFn.end())
        return mlir::failure();

    return linkOpToRuntimeFunc(it->second, op, operands, rewriter, 2);
}


mlir::LogicalResult CompareOpLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    pyir::CompareOp compareOp = mlir::cast<pyir::CompareOp>(op);

    // Map operator string to runtime function name
    static const std::unordered_map<std::string, std::string> opToFn = {
            {"==", "pyir_eq"}, {"!=", "pyir_ne"}, {"<", "pyir_lt"},
            {"<=", "pyir_le"}, {">", "pyir_gt"},  {">=", "pyir_ge"},
    };

    std::string opStr = compareOp.getOp().str();
    if (opStr.contains("bool(")) {
        opStr = opStr.substr(5); // Remove 'bool('
        opStr.pop_back(); // Remove ')'
    }
    const auto it = opToFn.find(opStr);
    if (it == opToFn.end())
        return mlir::failure();

    return linkOpToRuntimeFunc(it->second, op, operands, rewriter, 2);
}


mlir::LogicalResult FormatSimpleLowering::matchAndRewrite(mlir::Operation* op,
                                                          const mlir::ArrayRef<mlir::Value> operands,
                                                          mlir::ConversionPatternRewriter& rewriter) const {
    return linkOpToRuntimeFunc("pyir_formatSimple", op, operands, rewriter, 1);
}
