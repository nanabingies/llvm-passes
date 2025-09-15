#include "pass.hpp"

int main(int argc, char **argv) {
    llvm::LLVMContext Context;
    llvm::SMDiagnostic Err;

    // Load IR from file
    std::unique_ptr<llvm::Module> M = parseIRFile(llvm::StringRef(argv[1]), Err, Context);
    if (!M) {
        Err.print(argv[0], llvm::errs());
        return 1;
    }

    // Set up the pass manager
    llvm::PassBuilder PB;
    llvm::ModuleAnalysisManager MAM;
    PB.registerModuleAnalyses(MAM);

    llvm::ModulePassManager MPM;
    MPM.addPass(UVPass::CreateUninitializedVariablePass()); // Your pass here

    // Run the pass
    MPM.run(*M, MAM);

    // Output transformed IR
    M->print(llvm::outs(), nullptr);

    return 0;
}

// clang -S -emit-llvm test01.c -o test01.ll

/*
*   mkdir build
*   cd build
*   cmake ..
*   make
*/

