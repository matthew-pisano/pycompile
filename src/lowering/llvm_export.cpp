//
// Created by matthew on 3/9/26.
//

#include "lowering/llvm_export.h"

#include <mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h>
#include <mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h>
#include <mlir/Target/LLVMIR/Export.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Passes/PassBuilder.h>

#include <stdexcept>

/**
 * Initializes LLVM's native target, ASM printer, and ASM parser. Safe to call multiple times: LLVM initialization
 * is idempotent.
 */
static void initializeLLVMTargets() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}


/**
 * Creates an LLVM TargetMachine for the given options.
 *
 * The target machine controls code generation: architecture, CPU features, optimization level, and output format. It
 * is the central object used by both IR printing and object file emission.
 *
 * @param options Export options specifying target triple, CPU, and opt level.
 * @return An owning pointer to the created TargetMachine.
 * @throws std::runtime_error if the target triple cannot be resolved.
 */
static std::unique_ptr<llvm::TargetMachine> createTargetMachine(const LLVMExportOptions& options) {
    initializeLLVMTargets();

    const std::string tripleStr = options.targetTriple.empty()
                                      ? llvm::sys::getDefaultTargetTriple()
                                      : options.targetTriple;
    const llvm::Triple triple(llvm::Triple::normalize(tripleStr));

    const std::string cpu = options.cpu.empty() || options.cpu == "native"
                                ? llvm::sys::getHostCPUName().str()
                                : options.cpu;

    std::string err;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple.str(), err);
    if (!target)
        throw std::runtime_error("failed to lookup LLVM target: " + err);

    const llvm::TargetOptions targetOptions;
    const llvm::CodeGenOptLevel optLevel = static_cast<llvm::CodeGenOptLevel>(options.optLevel);

    llvm::TargetMachine* tm = target->createTargetMachine(
            triple, cpu, /*features=*/"", targetOptions,
            llvm::Reloc::PIC_, llvm::CodeModel::Small, optLevel);
    if (!tm)
        throw std::runtime_error("failed to create LLVM target machine");

    return std::unique_ptr<llvm::TargetMachine>(tm);
}

/**
 * Translates an MLIR module containing LLVM dialect ops into an llvm::Module. This is the bridge between the MLIR
 * world and the LLVM world, after this point everything is standard LLVM IR.
 *
 * @param mlirModule The MLIR module to translate.
 * @param llvmCtx The LLVM context to create the module in.
 * @return An owning pointer to the translated llvm::Module.
 * @throws std::runtime_error if translation fails.
 */
static std::unique_ptr<llvm::Module> translateToLLVM(mlir::ModuleOp mlirModule, llvm::LLVMContext& llvmCtx) {
    mlir::registerLLVMDialectTranslation(*mlirModule.getContext());
    mlir::registerBuiltinDialectTranslation(*mlirModule.getContext());

    std::unique_ptr<llvm::Module> llvmModule = mlir::translateModuleToLLVMIR(mlirModule, llvmCtx);
    if (!llvmModule)
        throw std::runtime_error("failed to translate MLIR module to LLVM IR");

    return llvmModule;
}

/**
 * Runs LLVM optimization passes on the module at the given optimization level. At opt level 0 this is a no-op.
 *
 * @param llvmModule The module to optimize, modified in place.
 * @param tm The target machine to use for target-specific passes.
 * @param optLevel Optimization level 0-3.
 */
static void optimizeLLVMModule(llvm::Module& llvmModule, llvm::TargetMachine& tm, const int unsigned optLevel) {
    if (optLevel == 0)
        return;

    const llvm::PipelineTuningOptions pto;
    llvm::PassBuilder pb(&tm, pto);

    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;

    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    llvm::OptimizationLevel level;
    switch (optLevel) {
        case 1:
            level = llvm::OptimizationLevel::O1;
            break;
        case 2:
            level = llvm::OptimizationLevel::O2;
            break;
        default:
            level = llvm::OptimizationLevel::O3;
            break;
    }

    llvm::ModulePassManager mpm = pb.buildPerModuleDefaultPipeline(level);
    mpm.run(llvmModule, mam);
}


void exportLLVMIR(const mlir::ModuleOp module, const LLVMExportOptions& options) {
    llvm::LLVMContext llvmCtx;
    const std::unique_ptr<llvm::Module> llvmModule = translateToLLVM(module, llvmCtx);

    const std::unique_ptr<llvm::TargetMachine> tm = createTargetMachine(options);
    llvmModule->setTargetTriple(llvm::Triple(tm->getTargetTriple().str()));
    llvmModule->setDataLayout(tm->createDataLayout());

    optimizeLLVMModule(*llvmModule, *tm, options.optLevel);

    llvmModule->print(llvm::outs(), nullptr);
    llvm::outs().flush();
}


void exportObjectFile(const mlir::ModuleOp module, const std::filesystem::path& output,
                      const LLVMExportOptions& options) {
    llvm::LLVMContext llvmCtx;
    const std::unique_ptr<llvm::Module> llvmModule = translateToLLVM(module, llvmCtx);

    const std::unique_ptr<llvm::TargetMachine> tm = createTargetMachine(options);
    llvmModule->setTargetTriple(llvm::Triple(tm->getTargetTriple().str()));
    llvmModule->setDataLayout(tm->createDataLayout());

    optimizeLLVMModule(*llvmModule, *tm, options.optLevel);

    // open output file
    std::error_code ec;
    llvm::raw_fd_ostream outStream(output.string(), ec, llvm::sys::fs::OF_None);
    if (ec)
        throw std::runtime_error("Failed to open output file: " + ec.message());

    // emit object file
    llvm::legacy::PassManager codegenPm;
    if (tm->addPassesToEmitFile(codegenPm, outStream, nullptr, llvm::CodeGenFileType::ObjectFile))
        throw std::runtime_error("Target machine cannot emit object files");

    codegenPm.run(*llvmModule);
    outStream.flush();
}
