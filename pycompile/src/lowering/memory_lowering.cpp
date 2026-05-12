//
// Created by matthew on 3/24/26.
//

#include "memory_lowering.h"

#include "pyir/pyir_attrs.h"


mlir::LogicalResult insertLoadRuntimeFunc(const std::string& func, const llvm::StringRef& varName, mlir::Operation* op,
                                          mlir::ConversionPatternRewriter& rewriter) {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = op->getParentOfType<mlir::ModuleOp>();

    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, func, fnType);

    const std::string globalName = "__pyir_str_" + varName.str();
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, op->getLoc(), globalName, varName);
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, op->getLoc(), fn, mlir::ValueRange{strPtr});
    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult insertStoreRuntimeFunc(const std::string& func, const llvm::StringRef& varName,
                                           const mlir::Value& value, mlir::Operation* op,
                                           mlir::ConversionPatternRewriter& rewriter) {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = op->getParentOfType<mlir::ModuleOp>();

    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, func, fnType);

    const std::string globalName = "__pyir_str_" + varName.str();
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, op->getLoc(), globalName, varName);
    mlir::LLVM::CallOp::create(rewriter, op->getLoc(), fn, mlir::ValueRange{strPtr, value});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult InitModuleLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                        mlir::ConversionPatternRewriter& rewriter) const {
    pyir::InitModule initModule = mlir::cast<pyir::InitModule>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_initModule(const char* file, const char* name)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_initModule", fnType);

    const std::string globalFile = "__pyir_str_" + initModule.getFile().str();
    const std::string globalName = "__pyir_str_" + initModule.getName().str();
    const mlir::Value filePtr = getOrInsertStringConstant(rewriter, module, loc, globalFile, initModule.getFile());
    const mlir::Value namePtr = getOrInsertStringConstant(rewriter, module, loc, globalName, initModule.getName());
    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{filePtr, namePtr});
    rewriter.eraseOp(op);
    return mlir::success();
}


mlir::LogicalResult DestroyModuleLowering::matchAndRewrite(mlir::Operation* op,
                                                           const mlir::ArrayRef<mlir::Value> operands,
                                                           mlir::ConversionPatternRewriter& rewriter) const {
    return insertRuntimeFunc("pyir_destroyModule", op, operands, rewriter, false);
}


mlir::LogicalResult LoadFastLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadFast loadFast = mlir::cast<pyir::LoadFast>(op);
    return insertLoadRuntimeFunc("pyir_loadFast", loadFast.getVarName(), op, rewriter);
}


mlir::LogicalResult LoadFastAndClearLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                              mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadFastAndClear loadFastAndClear = mlir::cast<pyir::LoadFastAndClear>(op);
    return insertLoadRuntimeFunc("pyir_loadFastAndClear", loadFastAndClear.getVarName(), op, rewriter);
}


mlir::LogicalResult StoreFastLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    pyir::StoreFast storeFast = mlir::cast<pyir::StoreFast>(op);
    return insertStoreRuntimeFunc("pyir_storeFast", storeFast.getVarName(), operands[0], op, rewriter);
}


mlir::LogicalResult LoadNameLowering::matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadName loadName = mlir::cast<pyir::LoadName>(op);
    return insertLoadRuntimeFunc("pyir_loadName", loadName.getVarName(), op, rewriter);
}


mlir::LogicalResult StoreNameLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                       mlir::ConversionPatternRewriter& rewriter) const {
    pyir::StoreName storeName = mlir::cast<pyir::StoreName>(op);
    return insertStoreRuntimeFunc("pyir_storeName", storeName.getVarName(), operands[0], op, rewriter);
}


mlir::Value LoadConstLowering::loadStringConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                               const mlir::Location& loc, const mlir::ModuleOp& module,
                                               const mlir::StringAttr& strAttr) {
    // declare: extern PyObj* pyir_loadConstStr(const char* str)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstStr", fnType);

    // create global string constant
    const std::string str = strAttr.getValue().str();
    const std::string gName = "__pyir_const_str_" + std::to_string(std::hash<std::string>{}(str));
    const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, gName, str);
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{strPtr});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadBoolConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                             const mlir::Location& loc, const mlir::ModuleOp& module,
                                             const mlir::BoolAttr& boolAttr) {
    // declare: extern PyObj* pyir_loadConstBool(int_8 val)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {boolType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstBool", fnType);
    mlir::LLVM::ConstantOp boolVal = mlir::LLVM::ConstantOp::create(
            rewriter, loc, boolType(ctx), rewriter.getI8IntegerAttr(boolAttr.getValue() ? 1 : 0));
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{boolVal});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadIntConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                            const mlir::Location& loc, const mlir::ModuleOp& module,
                                            const mlir::IntegerAttr& intAttr) {
    // declare: extern PyObj* pyir_loadConstInt(int64_t val)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstInt", fnType);
    mlir::LLVM::ConstantOp intVal =
            mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(intAttr.getInt()));
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{intVal});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadFloatConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                              const mlir::Location& loc, const mlir::ModuleOp& module,
                                              const mlir::FloatAttr& floatAttr) {
    // declare: extern PyObj* pyir_loadConstFloat(double_t val)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {f64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstFloat", fnType);
    mlir::LLVM::ConstantOp floatVal = mlir::LLVM::ConstantOp::create(
            rewriter, loc, f64Type(ctx), rewriter.getF64FloatAttr(floatAttr.getValueAsDouble()));
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{floatVal});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadNoneConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                             const mlir::Location& loc, const mlir::ModuleOp& module) {
    // declare: extern PyObj* pyir_loadConstNone()
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstNone", fnType);
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{});
    return call.getResult();
}


mlir::Value LoadConstLowering::loadTupleConst(mlir::ConversionPatternRewriter& rewriter, mlir::MLIRContext* ctx,
                                              const mlir::Location& loc, const mlir::ModuleOp& module,
                                              const mlir::ArrayAttr& arrAttr) {
    // declare: extern PyObj* pyir_loadConstTuple(Value** items, int64_t count)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadConstTuple", fnType);

    const int64_t count = static_cast<int64_t>(arrAttr.size());

    mlir::Value partsPtr;
    if (count > 0) {
        // Allocate stack array of PyObj*[count]
        const mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), count);
        mlir::LLVM::ConstantOp allocSize =
                mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(1));
        mlir::LLVM::AllocaOp alloca = mlir::LLVM::AllocaOp::create(rewriter, loc, ptrType(ctx), arrType, allocSize);

        // Materialize each element as a PyObj* and store into the array
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
                throw std::runtime_error("Unsupported element type in tuple");

            mlir::LLVM::ConstantOp idx =
                    mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));
            mlir::LLVM::GEPOp gep =
                    mlir::LLVM::GEPOp::create(rewriter, loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});
            mlir::LLVM::StoreOp::create(rewriter, loc, elemVal, gep);
        }
        partsPtr = alloca;
    } else
        partsPtr = mlir::LLVM::ZeroOp::create(rewriter, loc, ptrType(ctx));

    mlir::LLVM::ConstantOp countVal =
            mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(count));
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{partsPtr, countVal});
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
    const int64_t index = static_cast<int64_t>(loadArg.getIndex());

    // operands[0] is the args_ptr (Value** as !llvm.ptr)
    const mlir::Value argsPtr = operands[0];

    // GEP into args[index]
    mlir::LLVM::ConstantOp idx =
            mlir::LLVM::ConstantOp::create(rewriter, loc, i64Type(ctx), rewriter.getI64IntegerAttr(index));
    mlir::LLVM::GEPOp gep =
            mlir::LLVM::GEPOp::create(rewriter, loc, ptrType(ctx), ptrType(ctx), argsPtr, mlir::ValueRange{idx});

    // Load PyObj* from args[index]
    mlir::LLVM::LoadOp load = mlir::LLVM::LoadOp::create(rewriter, loc, ptrType(ctx), gep);

    rewriter.replaceOp(op, load.getResult());
    return mlir::success();
}


mlir::LogicalResult LoadAttrLowering::matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                                      mlir::ConversionPatternRewriter& rewriter) const {
    pyir::LoadAttr loadAttr = mlir::cast<pyir::LoadAttr>(op);
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern PyObj* pyir_loadAttr(PyObj* obj, const char* name)
    const mlir::LLVM::LLVMFunctionType fnType =
            mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_loadAttr", fnType);

    const std::string globalName = "__pyir_str_" + loadAttr.getAttrName().str();
    const mlir::Value namePtr = getOrInsertStringConstant(rewriter, module, loc, globalName, loadAttr.getAttrName());

    // operands[0] is the object
    mlir::LLVM::CallOp call = mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], namePtr});

    rewriter.replaceOp(op, call.getResult());
    return mlir::success();
}


mlir::LogicalResult StoreSubscrLowering::matchAndRewrite(mlir::Operation* op,
                                                         const mlir::ArrayRef<mlir::Value> operands,
                                                         mlir::ConversionPatternRewriter& rewriter) const {
    mlir::MLIRContext* ctx = op->getContext();
    const mlir::ModuleOp module = getModule(op);
    const mlir::Location loc = op->getLoc();

    // declare: extern void pyir_storeSubscr(PyObj* container, PyObj* idx, PyObj* value)
    const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
            mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx), ptrType(ctx)});
    const mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_storeSubscr", fnType);

    mlir::LLVM::CallOp::create(rewriter, loc, fn, mlir::ValueRange{operands[0], operands[1], operands[2]});
    rewriter.eraseOp(op);
    return mlir::success();
}
