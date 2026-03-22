//
// Created by matthew on 3/9/26.
//

#include "lowering/pyir_to_llvm.h"

#include <filesystem>

#include "pyir/pyir_ops.h"
#include "pyir/pyir_types.h"

#include <mlir/Conversion/LLVMCommon/ConversionTarget.h>
#include <mlir/Conversion/LLVMCommon/TypeConverter.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Transforms/DialectConversion.h>
#include <mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h>
#include <mlir/Conversion/ArithToLLVM/ArithToLLVM.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h>

#include <mlir/Transforms/Passes.h>

#include "utils.h"


/**
 * Registers a conversion from pyir::ByteCodeObjectType to !llvm.ptr.
 *
 * All Python values are represented as opaque pointers to heap-allocated
 * pyir::Value objects in the runtime. This avoids ABI complexity around
 * passing std::variant by value across the LLVM boundary.
 * @param converter The LLVM type converter
 */
static void addPyIRTypeConversions(mlir::LLVMTypeConverter& converter) {
    converter.addConversion([](const pyir::ByteCodeObjectType type) -> mlir::Type {
        return mlir::LLVM::LLVMPointerType::get(type.getContext());
    });
}


/**
 * Returns an opaque LLVM pointer type (!llvm.ptr) in the given context.
 * Used as the LLVM representation of pyir::Value* throughout the lowering.
 */
static mlir::LLVM::LLVMPointerType ptrType(mlir::MLIRContext* ctx) {
    return mlir::LLVM::LLVMPointerType::get(ctx);
}


/**
 * Returns a bool (8-bit integer for LLVM) type in the given context.
 */
static mlir::Type boolType(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 8);
}


/**
 * Returns a 64-bit integer type in the given context.
 */
static mlir::Type i64Type(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 64);
}


/**
 * Returns an 8-bit integer type in the given context.
 */
static mlir::Type i8Type(mlir::MLIRContext* ctx) {
    return mlir::IntegerType::get(ctx, 8);
}


/**
 * Returns a 64-bit float type in the given context.
 */
static mlir::Type f64Type(mlir::MLIRContext* ctx) {
    return mlir::Float64Type::get(ctx);
}


/**
 * Looks up an external runtime function by name in the module, inserting a declaration if one does not already exist.
 *
 * This is used to lazily declare runtime functions (e.g. pyir_load_name, pyir_call) only when an op that needs them is
 * encountered during lowering, avoiding unused declarations in the output.
 *
 * @param rewriter The pattern rewriter, used to insert the declaration.
 * @param module The module to insert the declaration into.
 * @param name The symbol name of the runtime function.
 * @param fnType The LLVM function type of the declaration.
 * @return The existing or newly created LLVMFuncOp declaration.
 */
static mlir::LLVM::LLVMFuncOp getOrInsertRuntimeFn(mlir::PatternRewriter& rewriter, mlir::ModuleOp module,
                                                   llvm::StringRef name, mlir::LLVM::LLVMFunctionType fnType) {
    if (const mlir::LLVM::LLVMFuncOp fn = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>(name))
        return fn;

    mlir::PatternRewriter::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(module.getBody());
    return rewriter.create<mlir::LLVM::LLVMFuncOp>(rewriter.getUnknownLoc(), name, fnType,
                                                   mlir::LLVM::Linkage::External);
}


/**
 * Creates a private global string constant in the module if one with the given name does not already exist, and
 * returns a pointer to it.
 *
 * The string is stored as a null-terminated i8 array in the module's rodata section. Identical strings are
 * deduplicated by name.
 *
 * @param rewriter The pattern rewriter, used to insert the global.
 * @param module The module to insert the global into.
 * @param loc The location to attach to the global op.
 * @param name The symbol name for the global.
 * @param str The string value to store, without null terminator.
 * @return An !llvm.ptr value pointing to the global.
 */
static mlir::Value getOrInsertStringConstant(mlir::ConversionPatternRewriter& rewriter, mlir::ModuleOp module,
                                             const mlir::Location loc, llvm::StringRef name,
                                             const llvm::StringRef str) {
    auto* ctx = module.getContext();

    if (!module.lookupSymbol(name)) {
        mlir::PatternRewriter::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(module.getBody());
        auto strType = mlir::LLVM::LLVMArrayType::get(i8Type(ctx), str.size() + 1);
        rewriter.create<mlir::LLVM::GlobalOp>(
                loc, strType, /*isConstant=*/true,
                mlir::LLVM::Linkage::Private, name,
                rewriter.getStringAttr(str.str() + '\0'));
    }

    return rewriter.create<mlir::LLVM::AddressOfOp>(loc, ptrType(ctx), name);
}


/**
 * Base class for all PyIR to LLVM conversion patterns.
 *
 * Provides a convenience method for retrieving the parent ModuleOp, which is needed by patterns that insert global
 * declarations or runtime function declarations.
 */
struct PyIROpConversion : mlir::ConversionPattern {
    PyIROpConversion(llvm::StringRef opName, const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        ConversionPattern(tc, opName, /*benefit=*/1, ctx) {
    }

    static mlir::ModuleOp getModule(mlir::Operation* op) {
        return op->getParentOfType<mlir::ModuleOp>();
    }

    /**
     * A helper function for directly linking ops to functions in the runtime standard library
     * @param func The name of the runtime function to link to
     * @param op The operation to be linked
     * @param operands The operands for the function
     * @param rewriter The MLIR rewriter
     * @param argc The number of arguments the op takes (e.g. binary or unary)
     * @return The status result
     */
    static mlir::LogicalResult linkOpToRuntimeFunc(const std::string& func, mlir::Operation* op,
                                                   const mlir::ArrayRef<mlir::Value> operands,
                                                   mlir::ConversionPatternRewriter& rewriter, const size_t argc) {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        mlir::LLVM::LLVMFunctionType fnType;
        if (argc == 1)
            fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
        else if (argc == 2)
            fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx), ptrType(ctx)});
        else
            throw std::runtime_error("Unsupported number of args in runtime function link");

        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, func, fnType);

        mlir::LLVM::CallOp call;
        if (argc == 1)
            call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0]});
        else
            call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0], operands[1]});

        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.is_truthy to a call to the runtime function pyir_is_truthy.
 *
 * Evaluates a heap-allocated Value* as a literal boolean.
 *
 * pyir.is_truthy %val : !pyir.object
 *     %result = llvm.call @pyir_is_truthy(%val)
 */
struct IsTruthyLowering : PyIROpConversion {
    IsTruthyLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::IsTruthy::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern int8_t pyir_is_truthy(Value* val)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                mlir::IntegerType::get(ctx, 8), {ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_is_truthy", fnType);

        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{operands[0]});

        // truncate i8 to i1 for cf.cond_br
        const mlir::Value i1val = rewriter.create<mlir::LLVM::TruncOp>(
                loc, mlir::IntegerType::get(ctx, 1), call.getResult());

        rewriter.replaceOp(op, i1val);
        return mlir::success();
    }
};


/**
 * Lowers pyir.to_bool to a call to the runtime function pyir_to_bool.
 *
 * Coerces a heap-allocated Value* to a boolean Value* by delegating to the runtime toBool helper. The result is a
 * freshly allocated Value(bool).
 *
 * pyir.to_bool %val : !pyir.object
 *     %result = llvm.call @pyir_to_bool(%val)
 */
struct ToBoolLowering : PyIROpConversion {
    ToBoolLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::ToBool::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        return linkOpToRuntimeFunc("pyir_to_bool", op, operands, rewriter, 1);
    }
};


/**
 * Lowers pyir.pyir_unary_invert to a call to the runtime function pyir_unary_invert.
 *
 * Inverts a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unary_invert %val : !pyir.object
 *     %result = llvm.call @pyir_unary_invert(%val)
 */
struct UnaryInvertLowering : PyIROpConversion {
    UnaryInvertLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryInvert::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        return linkOpToRuntimeFunc("pyir_unary_invert", op, operands, rewriter, 1);
    }
};


/**
 * Lowers pyir.pyir_unary_negative to a call to the runtime function pyir_unary_negative.
 *
 * Negates a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unary_negative %val : !pyir.object
 *     %result = llvm.call @pyir_unary_negative(%val)
 */
struct UnaryNegativeLowering : PyIROpConversion {
    UnaryNegativeLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryNegative::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        return linkOpToRuntimeFunc("pyir_unary_negative", op, operands, rewriter, 1);
    }
};


/**
 * Lowers pyir.pyir_unary_not to a call to the runtime function pyir_unary_not.
 *
 * Negates a heap-allocated Value* and returns a new heap-allocated Value*.
 *
 * pyir.pyir_unary_not %val : !pyir.object
 *     %result = llvm.call @pyir_unary_not(%val)
 */
struct UnaryNotLowering : PyIROpConversion {
    UnaryNotLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::UnaryNot::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        return linkOpToRuntimeFunc("pyir_unary_not", op, operands, rewriter, 1);
    }
};


/**
 * Lowers pyir.binary_op to a call to the appropriate runtime binary operator function.
 *
 * The operator string is mapped to a runtime function at compile time. Both operands are heap-allocated Value*
 * pointers. The runtime performs the operation and returns a new heap-allocated Value*.
 *
 * pyir.binary_op "+", %lhs, %rhs
 *     %result = llvm.call @pyir_add(%lhs, %rhs)
 */
struct BinaryOpLowering : PyIROpConversion {
    BinaryOpLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BinaryOp::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::BinaryOp binaryOp = mlir::cast<pyir::BinaryOp>(op);

        // Map operator string to runtime function name
        static const std::unordered_map<std::string, std::string> opToFn = {
                {"+", "pyir_add"},
                {"-", "pyir_sub"},
                {"*", "pyir_mul"},
                {"/", "pyir_div"},
                {"^", "pyir_xor"},
        };

        const std::string opStr = binaryOp.getOp().str();
        const auto it = opToFn.find(opStr);
        if (it == opToFn.end())
            return mlir::failure();

        return linkOpToRuntimeFunc(it->second, op, operands, rewriter, 2);
    }
};


/**
 * Lowers pyir.compare_op to a call to the appropriate runtime compare operator function.
 *
 * The operator string is mapped to a runtime function at compile time. Both operands are heap-allocated Value*
 * pointers. The runtime performs the operation and returns a new heap-allocated Value*.
 *
 * pyir.compare_op "==", %lhs, %rhs
 *     %result = llvm.call @pyir_eq(%lhs, %rhs)
 */
struct CompareOpLowering : PyIROpConversion {
    CompareOpLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::CompareOp::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::CompareOp compareOp = mlir::cast<pyir::CompareOp>(op);

        // Map operator string to runtime function name
        static const std::unordered_map<std::string, std::string> opToFn = {
                {"==", "pyir_eq"},
                {"!=", "pyir_ne"},
                {"<", "pyir_lt"},
                {"<=", "pyir_le"},
                {">", "pyir_gt"},
                {">=", "pyir_ge"},
        };

        const std::string opStr = compareOp.getOp().str();
        const auto it = opToFn.find(opStr);
        if (it == opToFn.end())
            return mlir::failure();

        return linkOpToRuntimeFunc(it->second, op, operands, rewriter, 2);
    }
};


/**
 * Lowers pyir.load_fast to a call to the runtime function pyir_load_fast.
 *
 * The name string is stored as a constant and passed as a const char* pointer. The runtime resolves the name
 * against the names present in the current scope and returns a heap-allocated Value*.
 *
 * pyir.load_fast "arg"
 *     %ptr = llvm.mlir.addressof @__pyir_str_arg
  *    %val = llvm.call @pyir_load_fast(%ptr)
 */
struct LoadFastLowering : PyIROpConversion {
    LoadFastLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadFast::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::LoadFast loadFast = mlir::cast<pyir::LoadFast>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern Value* pyir_load_fast(const char* name)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_fast", fnType);

        const std::string globalName = "__pyir_str_" + loadFast.getVarName().str();
        const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, loadFast.getVarName());
        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});
        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.store_fast to a call to the runtime function pyir_store_fast.
 *
 * The name string is stored as a constant and passed as a const char* pointer alongside the heap-allocated
 * Value* to store. The runtime inserts or replaces the name in a local scope table, managing refcounts on the old
 * and new values.
 *
 * pyir.store_fast "x", %val
 *   %ptr = llvm.mlir.addressof @__pyir_str_x
 *          llvm.call @pyir_store_fast(%ptr, %val)
 */
struct StoreFastLowering : PyIROpConversion {
    StoreFastLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::StoreFast::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::StoreFast storeFast = mlir::cast<pyir::StoreFast>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern void pyir_store_fast(const char* name, Value* val)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_store_fast", fnType);

        const std::string globalName = "__pyir_str_" + storeFast.getVarName().str();
        const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, storeFast.getVarName());
        rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr, operands[0]});
        rewriter.eraseOp(op);
        return mlir::success();
    }
};


/**
 * Lowers pyir.load_name to a call to the runtime function pyir_load_name.
 *
 * The name string is stored as a global constant and passed as a const char* pointer. The runtime resolves the name
 * against the builtin table or module names and returns a heap-allocated Value*.
 *
 * pyir.load_name "print"
 *     %ptr = llvm.mlir.addressof @__pyir_str_print
 *     %val = llvm.call @pyir_load_name(%ptr)
 */
struct LoadNameLowering : PyIROpConversion {
    LoadNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadName::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::LoadName loadName = mlir::cast<pyir::LoadName>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern Value* pyir_load_name(const char* name)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_name", fnType);

        // create a global string constant for the name
        const std::string globalName = "__pyir_str_" + loadName.getVarName().str();

        // get pointer to the global string
        const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, loadName.getVarName());

        // call pyir_load_name
        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});

        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.store_name to a call to the runtime function pyir_store_name.
 *
 * The name string is stored as a global constant and passed as a const char* pointer alongside the heap-allocated
 * Value* to store. The runtime inserts or replaces the name in the module scope table, managing refcounts on the old
 * and new values.
 *
 * pyir.store_name "a", %val
 *     %ptr = llvm.mlir.addressof @__pyir_str_a
 *            llvm.call @pyir_store_name(%ptr, %val)
 */
struct StoreNameLowering : PyIROpConversion {
    StoreNameLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::StoreName::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::StoreName storeName = mlir::cast<pyir::StoreName>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern void pyir_store_name(const char* name, Value* val)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx), ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_store_name", fnType);

        const std::string globalName = "__pyir_str_" + storeName.getVarName().str();
        const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, globalName, storeName.getVarName());

        rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr, operands[0]});
        rewriter.eraseOp(op);
        return mlir::success();
    }
};


/**
 * Lowers pyir.load_const to a call to the appropriate runtime constant constructor, depending on the attribute type:
 *
 *   StringAttr -> pyir_load_const_str(const char*)
 *   IntegerAttr -> pyir_load_const_int(int64_t)
 *
 * String constants are stored as global i8 arrays. Integer constants are passed directly as i64 values. Both return a
 * heap-allocated Value*.
 */
struct LoadConstLowering : PyIROpConversion {
    LoadConstLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::LoadConst::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        pyir::LoadConst loadConst = mlir::cast<pyir::LoadConst>(op);
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        mlir::Value result;

        // String constant type
        if (const mlir::StringAttr strAttr = mlir::dyn_cast<mlir::StringAttr>(loadConst.getValue())) {
            // declare: extern Value* pyir_load_const_str(const char* str)
            const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {ptrType(ctx)});
            mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_const_str", fnType);

            // create global string constant
            const std::string str = strAttr.getValue().str();
            const std::string gName = "__pyir_const_str_" + std::to_string(std::hash<std::string>{}(str));
            const mlir::Value strPtr = getOrInsertStringConstant(rewriter, module, loc, gName, str);
            mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{strPtr});
            result = call.getResult();
        }
        // Bool constant type
        else if (const mlir::BoolAttr boolAttr = mlir::dyn_cast<mlir::BoolAttr>(loadConst.getValue())) {
            // declare: extern Value* pyir_load_const_bool(int_8 val)
            const mlir::LLVM::LLVMFunctionType fnType =
                    mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {boolType(ctx)});
            mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_const_bool", fnType);
            mlir::LLVM::ConstantOp boolVal = rewriter.create<mlir::LLVM::ConstantOp>(
                    loc, boolType(ctx), rewriter.getI8IntegerAttr(boolAttr.getValue() ? 1 : 0));
            mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{boolVal});
            result = call.getResult();
        }
        // Int constant type
        else if (const mlir::IntegerAttr intAttr = mlir::dyn_cast<mlir::IntegerAttr>(loadConst.getValue())) {
            // declare: extern Value* pyir_load_const_int(int64_t val)
            const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {i64Type(ctx)});
            mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_const_int", fnType);
            mlir::LLVM::ConstantOp intVal = rewriter.create<mlir::LLVM::ConstantOp>(
                    loc, i64Type(ctx), rewriter.getI64IntegerAttr(intAttr.getInt()));
            mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{intVal});
            result = call.getResult();
        }
        // Float constant type
        else if (const mlir::FloatAttr floatAttr = mlir::dyn_cast<mlir::FloatAttr>(loadConst.getValue())) {
            // declare: extern Value* pyir_load_const_float(double_t val)
            const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(ptrType(ctx), {f64Type(ctx)});
            mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_load_const_float", fnType);
            mlir::LLVM::ConstantOp floatVal = rewriter.create<mlir::LLVM::ConstantOp>(
                    loc, f64Type(ctx), rewriter.getF64FloatAttr(floatAttr.getValueAsDouble()));
            mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{floatVal});
            result = call.getResult();
        }
        // Unhandled constant type
        else
            return mlir::failure();

        rewriter.replaceOp(op, result);
        return mlir::success();
    }
};


/**
 * Lowers pyir.push_null to a call to pyir_push_null().
 *
 * push_null is a CPython calling convention artifact that places a null sentinel below the callee on the stack. In the
 * lowered IR it becomes a runtime call that returns a null Value* sentinel, which the Call lowering discards.
 */
struct PushNullLowering : PyIROpConversion {
    PushNullLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PushNull::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, mlir::ArrayRef<mlir::Value>,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        rewriter.replaceOpWithNewOp<mlir::LLVM::ZeroOp>(op, ptrType(op->getContext()));
        return mlir::success();
    }
};


/**
 * Lowers pyir.call to a call to the runtime function pyir_call.
 *
 * Arguments are stack-allocated as a Value*[] array and passed by pointer along with the argument count. The callee is
 * passed as a Value* pointer.
 *
 * pyir.call %callee(%arg0, %arg1)
 *     %arr    = llvm.alloca [2 x !llvm.ptr]
 *     %gep0   = llvm.gep %arr[0]
 *               llvm.store %arg0, %gep0
 *     %gep1   = llvm.gep %arr[1]
 *               llvm.store %arg1, %gep1
 *     %result = llvm.call @pyir_call(%callee, %arr, 2)
 *
 * For zero-argument calls, a null pointer is passed instead of an array.
 */
struct CallLowering : PyIROpConversion {
    CallLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::Call::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // operands[0] = callee, operands[1..] = args (after type conversion)
        const mlir::Value callee = operands[0];
        const mlir::ArrayRef<mlir::Value> args = operands.drop_front(1);
        const int64_t argc = static_cast<int64_t>(args.size());

        // declare: extern Value* pyir_call(Value* callee, Value** args, int64_t argc)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                ptrType(ctx), {ptrType(ctx), ptrType(ctx), i64Type(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_call", fnType);

        // allocate args array on the stack: Value*[argc]
        mlir::Value argsPtr;
        if (argc > 0) {
            mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), argc);
            mlir::LLVM::AllocaOp alloca = rewriter.create<mlir::LLVM::AllocaOp>(
                    loc, ptrType(ctx), arrType,
                    rewriter.create<mlir::LLVM::ConstantOp>(loc, i64Type(ctx), rewriter.getI64IntegerAttr(1)));

            // store each arg into the array
            for (int64_t i = 0; i < argc; i++) {
                mlir::LLVM::ConstantOp idx = rewriter.create<mlir::LLVM::ConstantOp>(
                        loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));
                mlir::LLVM::GEPOp gep = rewriter.create<mlir::LLVM::GEPOp>(
                        loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});
                rewriter.create<mlir::LLVM::StoreOp>(loc, args[i], gep);
            }
            argsPtr = alloca;
        } else
        // null pointer for empty args
            argsPtr = rewriter.create<mlir::LLVM::ZeroOp>(loc, ptrType(ctx));

        mlir::LLVM::ConstantOp argcVal = rewriter.create<mlir::LLVM::ConstantOp>(
                loc, i64Type(ctx), rewriter.getI64IntegerAttr(argc));

        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(
                loc, fn, mlir::ValueRange{callee, argsPtr, argcVal});

        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * Lowers pyir.pop_top by erasing it.
 *
 * pop_top discards the top of the Python evaluation stack. In SSA form the value simply has no uses, so the op itself
 * can be erased. DCE will subsequently remove the producing op if it is Pure and has no other uses.
 */
struct PopTopLowering : PyIROpConversion {
    PopTopLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::PopTop::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                mlir::LLVM::LLVMVoidType::get(ctx), {ptrType(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_decref", fnType);

        rewriter.create<mlir::LLVM::CallOp>(loc, fn, operands[0]);
        rewriter.eraseOp(op);
        return mlir::success();
    }
};


/**
 * Lowers pyir.pyir_format_simple to a call to the runtime function pyir_format_simple.
 *
 * Formats a heap-allocated Value* as a string and returns a new heap-allocated Value*.
 *
 * pyir.pyir_format_simple %val : !pyir.object
 *     %result = llvm.call @pyir_format_simple(%val)
 */
struct FormatSimpleLowering : PyIROpConversion {
    FormatSimpleLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::FormatSimple::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        return linkOpToRuntimeFunc("pyir_format_simple", op, operands, rewriter, 1);
    }
};


/**
 * Lowers pyir.pyir_build_string to allocate memory and construct a new string using the runtime function pyir_build_string
 *
 * Parts are stack-allocated as a Value*[] array and passed by pointer along with the part count.
 * The runtime concatenates all parts into a single string Value*.
 *
 * pyir.build_string %part0, %part1, ... : (!pyir.object, !pyir.object, ...) -> !pyir.object
 *     %arr   = llvm.alloca [n x !llvm.ptr]
 *     %gep0  = llvm.gep %arr[0]
 *              llvm.store %part0, %gep0
 *     %gep1  = llvm.gep %arr[1]
 *              llvm.store %part1, %gep1
 *     ...
 *     %result = llvm.call @pyir_build_string(%arr, n)
 */
struct BuildStringLowering : PyIROpConversion {
    BuildStringLowering(const mlir::LLVMTypeConverter& tc, mlir::MLIRContext* ctx) :
        PyIROpConversion(pyir::BuildString::getOperationName(), tc, ctx) {
    }

    mlir::LogicalResult matchAndRewrite(mlir::Operation* op, const mlir::ArrayRef<mlir::Value> operands,
                                        mlir::ConversionPatternRewriter& rewriter) const override {
        mlir::MLIRContext* ctx = op->getContext();
        const mlir::ModuleOp module = getModule(op);
        const mlir::Location loc = op->getLoc();

        // declare: extern Value* pyir_build_string(Value** parts, int64_t count)
        const mlir::LLVM::LLVMFunctionType fnType = mlir::LLVM::LLVMFunctionType::get(
                ptrType(ctx), {ptrType(ctx), i64Type(ctx)});
        mlir::LLVM::LLVMFuncOp fn = getOrInsertRuntimeFn(rewriter, module, "pyir_build_string", fnType);

        const int64_t count = static_cast<int64_t>(operands.size());

        // Allocate a stack array of Value* to hold all string parts: Value*[count]
        mlir::LLVM::LLVMArrayType arrType = mlir::LLVM::LLVMArrayType::get(ptrType(ctx), count);
        mlir::LLVM::ConstantOp allocSize = rewriter.create<mlir::LLVM::ConstantOp>(
                loc, i64Type(ctx), rewriter.getI64IntegerAttr(1));
        mlir::LLVM::AllocaOp alloca = rewriter.create<mlir::LLVM::AllocaOp>(loc, ptrType(ctx), arrType, allocSize);

        // Store each string part into the array in order
        for (int64_t i = 0; i < count; i++) {
            // Compute pointer to parts[i] via GEP (get element pointer)
            mlir::LLVM::ConstantOp idx = rewriter.create<mlir::LLVM::ConstantOp>(
                    loc, i64Type(ctx), rewriter.getI64IntegerAttr(i));

            mlir::LLVM::GEPOp gep = rewriter.create<mlir::LLVM::GEPOp>(
                    loc, ptrType(ctx), ptrType(ctx), alloca, mlir::ValueRange{idx});

            // Store the i-th operand (a Value*) into parts[i]
            rewriter.create<mlir::LLVM::StoreOp>(loc, operands[i], gep);
        }

        // Pass the array pointer and count to pyir_build_string
        mlir::LLVM::ConstantOp countVal = rewriter.create<mlir::LLVM::ConstantOp>(
                loc, i64Type(ctx), rewriter.getI64IntegerAttr(count));
        mlir::LLVM::CallOp call = rewriter.create<mlir::LLVM::CallOp>(loc, fn, mlir::ValueRange{alloca, countVal});

        rewriter.replaceOp(op, call.getResult());
        return mlir::success();
    }
};


/**
 * MLIR pass that lowers the entire PyIR dialect to the LLVM dialect.
 *
 * Applies all PyIR to LLVM conversion patterns along with the standard func to LLVM patterns. Marks the PyIR dialect
 * as illegal and the LLVM dialect as legal, so applyFullConversion will fail if any PyIR ops remain after the patterns
 * have been applied.
 */
struct PyIRToLLVMPass : mlir::PassWrapper<PyIRToLLVMPass, mlir::OperationPass<mlir::ModuleOp> > {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(PyIRToLLVMPass)

    void getDependentDialects(mlir::DialectRegistry& registry) const override {
        registry.insert<mlir::LLVM::LLVMDialect>();
    }

    void runOnOperation() override {
        const mlir::ModuleOp module = getOperation();
        mlir::MLIRContext* ctx = &getContext();

        mlir::LLVMTypeConverter typeConverter(ctx);
        addPyIRTypeConversions(typeConverter);

        mlir::RewritePatternSet patterns(ctx);
        populatePyIRToLLVMPatterns(patterns, typeConverter);
        mlir::populateFuncToLLVMConversionPatterns(typeConverter, patterns);
        mlir::arith::populateArithToLLVMConversionPatterns(typeConverter, patterns);
        mlir::cf::populateControlFlowToLLVMConversionPatterns(typeConverter, patterns);

        mlir::LLVMConversionTarget target(*ctx);
        target.addIllegalDialect<pyir::PyIRDialect>();
        target.addIllegalDialect<mlir::arith::ArithDialect>();
        target.addLegalDialect<mlir::LLVM::LLVMDialect>();
        target.addLegalOp<mlir::ModuleOp>();

        if (mlir::failed(mlir::applyFullConversion(module, target, std::move(patterns))))
            signalPassFailure();
    }
};


void populatePyIRToLLVMPatterns(mlir::RewritePatternSet& patterns,
                                mlir::LLVMTypeConverter& typeConverter) {
    mlir::MLIRContext* ctx = patterns.getContext();
    patterns.add<
        IsTruthyLowering,
        ToBoolLowering,
        UnaryNegativeLowering,
        UnaryNotLowering,
        UnaryInvertLowering,
        BinaryOpLowering,
        CompareOpLowering,
        LoadNameLowering,
        StoreNameLowering,
        LoadConstLowering,
        PushNullLowering,
        CallLowering,
        PopTopLowering,
        FormatSimpleLowering,
        BuildStringLowering
    >(typeConverter, ctx);
}


std::unique_ptr<mlir::Pass> createPyIRToLLVMPass() {
    return std::make_unique<PyIRToLLVMPass>();
}


void lowerToLLVMDialect(mlir::MLIRContext& ctx, const mlir::OwningOpRef<mlir::ModuleOp>& module) {
    ctx.loadDialect<mlir::LLVM::LLVMDialect>();

    mlir::PassManager pm(&ctx);

    pm.addPass(mlir::createCanonicalizerPass());
    // Lower PyIR to LLVM dialect
    pm.addPass(createPyIRToLLVMPass());

    mlir::Location errorLoc = mlir::UnknownLoc::get(&ctx);
    std::string errorMessage;
    mlir::ScopedDiagnosticHandler handler(&ctx, [&](const mlir::Diagnostic& diag) {
        if (diag.getSeverity() == mlir::DiagnosticSeverity::Error) {
            errorMessage = diag.str();
            errorLoc = diag.getLocation();
        }
        return mlir::success();
    });

    const llvm::LogicalResult result = pm.run(module.get());
    if (mlir::failed(result)) {
        size_t lineno = 0;
        size_t offset = 0;
        std::string moduleName = "<unknown>";
        if (const mlir::FileLineColLoc fileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(errorLoc)) {
            moduleName = std::filesystem::path(fileLoc.getFilename().str()).filename();
            lineno = fileLoc.getLine();
            offset = fileLoc.getColumn();
        }
        throw PyCompileError(errorMessage, moduleName, lineno, offset);
    }
}
