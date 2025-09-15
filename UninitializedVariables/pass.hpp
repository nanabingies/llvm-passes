#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace UVPass {

    struct UninitializedVariablePass : public llvm::PassInfoMixin<UninitializedVariablePass> {
        llvm::PreservedAnalyses run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM);

        static bool isRequired() {
            return true;
        }
    };
    
    inline llvm::ModulePassManager CreateUninitializedVariablePass() {
        llvm::ModulePassManager MPM;
        MPM.addPass(UninitializedVariablePass());
        return MPM;
    }
};   // namespace Pass

