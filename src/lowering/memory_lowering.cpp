//
// Created by matthew on 3/24/26.
//

#include "lowering/memory_lowering.h"
#include "pyir/pyir_attrs.h"


mlir::LogicalResult LoadFastLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadFast loadFast = mlir::cast<pyir::LoadFast>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern Value* pyir_loadFast(const char* name)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadFast", fnType);

    const std::string globalName = "__pyir_str_" + loadFast.getVarName().str();
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, loadFast.getVarName());
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});
    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult StoreFastLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    pyir::StoreFast storeFast = mlir::cast<pyir::StoreFast>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_storeFast(const char* name, Value* val)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_storeFast", fnType);

    const std::string globalName = "__pyir_str_" + storeFast.getVarName().str();
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, storeFast.getVarName());
    rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr, operands[0]});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult LoadNameLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadName loadName = mlir::cast<pyir::LoadName>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern Value* pyir_loadName(const char* name)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadName", fnType);

    const std::string globalName = "__pyir_str_" + loadName.getVarName().str();
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, loadName.getVarName());
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});
    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult StoreNameLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    pyir::StoreName storeName = mlir::cast<pyir::StoreName>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_storeName(const char* name, Value* val)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_storeName", fnType);

    const std::string globalName = "__pyir_str_" + storeName.getVarName().str();
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, storeName.getVarName());
    rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr, operands[0]});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::Value LoadConstLowering::loadStringConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                               const mlir::Location& loc, const mlir::ModuleOp& module,
                                               const mlir::StringAttr& strAttr) {
    // declare: extern Value* pyir_loadConstStr(const char* str)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstStr", fnType);

    // create global string constant
    const std::string str = strAttr.getValue().str();
    const std::string gName = "__pyir_const_str_" + std::to_string(std::hash<std::string>{}(str));
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, gName, str);
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadBoolConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                             const mlir::Location& loc, const mlir::ModuleOp& module,
                                             const mlir::BoolAttr& boolAttr) {
    // declare: extern Value* pyir_loadConstBool(int_8 val)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {boolType(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstBool", fnType);
    mlir::LLVM::ConstantOp boolVal = rewriter.create<mlir::LLVM::ConstantOp>(
            loc, boolType(ctx), rewriter.getI8IntegerAttr(boolAttr.getValue() ? 1 : 0));
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{boolVal});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadIntConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                            const mlir::Location& loc, const mlir::ModuleOp& module,
                                            const mlir::IntegerAttr& intAttr) {
    // declare: extern Value* pyir_loadConstInt(int64_t val)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {i64Type(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstInt", fnType);
    mlir::LLVM::ConstantOp intVal =
            rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(intAttr.getInt()));
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{intVal});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadFloatConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                              const mlir::Location& loc, const mlir::ModuleOp& module,
                                              const mlir::FloatAttr& floatAttr) {
    // declare: extern Value* pyir_loadConstFloat(double_t val)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {f64Type(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstFloat", fnType);
    mlir::LLVM::ConstantOp floatVal = rewriter.create<mlir::LLVM::ConstantOp>(
            loc, f64Type(ctx), rewriter.getF64FloatAttr(floatAttr.getValueAsDouble()));
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{floatVal});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadNoneConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                             const mlir::Location& loc, const mlir::ModuleOp& module) {
    // declare: extern Value* pyir_loadConstNone()
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstNone", fnType);
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadTupleConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                              const mlir::Location& loc, const mlir::ModuleOp& module,
                                              const mlir::ArrayAttr& arrAttr) {
    // declare: extern Value* pyir_loadConstTuple(Value** items, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstTuple", fnType);

    const int64_t count = static_cast<int64_t>(arrAttr.size());

    mlir::Value partsPtr;
    if (count > 0) {
        // Allocate stack array of Value*[count]
        mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), count);
        mlir::LLVM::ConstantOp allocSize =
                rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(1));
        mlir::LLVM::AllocaOp alloca = rewriter.create<mlir::LLVM::AllocaOp>(loc, ptrType(ctx), arrType, allocSize);

        // Materialize each element as a Value* and store into the array
        for (int64_t i = 0; i < count; i++) {
            mlir::Attribute elemAttr = arrAttr[i];
            mlir::Value elemVal;

            if (auto strAttr = mlir::dyn_cast<mlir::StringAttr>(elemAttr))
                elemVal = loadStringConst(rewriter, ctx, loc, module, strAttr);
            else if (auto boolAttr = mlir::dyn_cast<mlir::BoolAttr>(elemAttr))
                elemVal = loadBoolConst(rewriter, ctx, loc, module, boolAttr);
            else if (auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(elemAttr))
                elemVal = loadIntConst(rewriter, ctx, loc, module, intAttr);
            else if (auto floatAttr = mlir::dyn_cast<mlir::FloatAttr>(elemAttr))
                elemVal = loadFloatConst(rewriter, ctx, loc, module, floatAttr);
            else
                return {}; // unsupported element type

            mlir::LLVM::ConstantOp idx =
                    rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));
            mlir::LLVM::GEPOp gep =
                    rewriter.create<mlir::LLVM::GEPOp>(loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});
            rewriter.create<mlir::LLVM::StoreOp>(loc, elemVal, gep);
        }
        partsPtr = alloca;
    } else
        partsPtr = rewriter.create<mlir::LLVM::ZeroOp>(loc, ptrType(ctx));

    mlir::LLVM::ConstantOp countVal =
            rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(count));
    mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{partsPtr, countVal});
    return call.getResult();
}


mlir::LogicalResult LoadConstLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadConst loadConst = mlir::cast<pyir::LoadConst>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::Location loc = op->getLoc();
    const mlir::ModuleOp module = getModule(op);

    mlir::Value result;

    if (const mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue()))
        result = loadStringConst(rewriter, ctx, loc, module, strAttr);
    else if (const mlir::BoolAttr boolAttr = mlir::dyn_cast<mlir::BoolAttr>(loadConst.getValue()))
        result = loadBoolConst(rewriter, ctx, loc, module, boolAttr);
    else if (const mlir::IntegerAttr intAttr = mlir::dyn_cast<mlir::IntegerAttr>(loadConst.getValue()))
        result = loadIntConst(rewriter, ctx, loc, module, intAttr);
    else if (const mlir::FloatAttr floatAttr = mlir::dyn_cast<mlir::FloatAttr>(loadConst.getValue()))
        result = loadFloatConst(rewriter, ctx, loc, module, floatAttr);
    else if (mlir::isa<pyir::NoneAttr>(loadConst.getValue()))
        result = loadNoneConst(rewriter, ctx, loc, module);
    else if (const mlir::ArrayAttr arrAttr = mlir::dyn_cast<mlir::ArrayAttr>(loadConst.getValue()))
        result = loadTupleConst(rewriter, ctx, loc, module, arrAttr);
    else
        return mlir::failure();

    rewriter.replaceOp(op, result);
    return mlir::success();
}


mlir::LogicalResult LoadArgLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                     mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::Location loc = op->getLoc();

    pyir::LoadArg loadArg = mlir::cast<pyir::LoadArg>(op);
    const int64_t index = loadArg.getIndex();

    // operands[0] is the args_ptr (Value** as !llvm.ptr)
    mlir::Value argsPtr = operands[0];

    // GEP into args[index]
    mlir::LLVM::ConstantOp idx =
            rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(index));
    mlir::LLVM::GEPOp gep =
            rewriter.create<mlir::LLVM::GEPOp>(loc, ptrType(ctx), ptrType(ctx), argsPtr, mlir::ValueRange{idx});

    // Load Value* from args[index]
    mlir::LLVM::LoadOp load = rewriter.create<mlir::LLVM::LoadOp>(loc, ptrType(ctx), gep);

    rewriter.replaceOp(op, load.getResult());
    return mlir::success();
}
