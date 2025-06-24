/*
*   https://github.com/hsingwenhsu/ControlFlow-Analysis
*   
*   Unreachable Function Identification and Elimination with Control Flow Analysis
*
*   Task 1: Identification of Dead Functions
*   In this task, you will first implement the getCallGraph to extract the call graph. 
*   This function takes a vector of Function * containing all the functions in the current Module, 
*   and you need to create a hash map (a common way to store graph structure) for the call graph. 
*   You will then implement the getDeadFunctions function and return a vector of Function * denoting the unreachable/dead 
*   functions (i.e., functions that are not reachable from main). 
*   This function takes in a vector<Function *> containing all functions in the module, map<Function *, vector<Function *>> 
*   containing the call graph, and a Function * indication the pointer to the entry function. 
*   The TODO comment also summarizes what needs to be done.
*
*   Task 2: Removal of Dead Functions
*   Once you find all the dead functions, you will implement the removeDeadFunctions function to actually perform the 
*   removal of all these dead functions. 
*   Note that our definition of "dead" does NOT mean a function is not called from anywhere in the code. 
*   It simply refers to the fact that a function is not reachable from main. 
*   Thus, while you are removing a dead function, make sure that you properly update any possible call sites of that function. 
*   Given this hint, you will probably need to do some research to find suitable APIs to accomplish this task.
*/

/*
*   Steps I'm using to solve this problem.
*   1. Get the main function.
*   2. Extracting the Call Graph:
*       Implement getCallGraph to construct a hash map representing function calls.
*       Each node represents a function, and edges indicate direct calls between functions.
*   3. Identifying Dead Functions:
*       Implement getDeadFunctions to find functions that are not reachable from main.
*       This requires traversing the call graph and marking reachable functions.
*   4. Removing Dead Functions:
*       Implement removeDeadFunctions to eliminate unreachable functions.
*       Ensure proper updates to call sites to maintain correctness.
*/

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
    static llvm::Function* MainFunction{};
    static llvm::DenseMap<llvm::StringRef, std::vector<llvm::Function*>> CallGraph{};
    static std::vector<llvm::Function*> DeadFunctions{};

    void GetMainFunction(llvm::Module& Module) {
        std::string str = "main";

        for (auto& Function : Module) {
            if (Function.getName() == str) {
                MainFunction = &Function;
                break;
            }
        }

        return;
    }

    void RemoveDeadFunctionsCFG(llvm::Module& Module) {
        for (const auto& Function : DeadFunctions) {
            Function->replaceAllUsesWith(llvm::UndefValue::get(Function->getType()));
            Function->eraseFromParent();
        }
    }

    std::vector<llvm::Function*> GetCallsInFunction(llvm::Function* TheFunction) {
        std::vector<llvm::Function*> CallFunctions{};

        auto& EntryBB = TheFunction->getEntryBlock();

        std::set<llvm::BasicBlock*> visitedBBs{};
        std::stack<llvm::BasicBlock*> stackBBs{};
        stackBBs.push(&EntryBB);

        do {
            auto CurrBB = stackBBs.top();
            stackBBs.pop();

            if (visitedBBs.find(CurrBB) != visitedBBs.end())
                continue;

            visitedBBs.insert(CurrBB);

            // Get call instructions in current BasicBlock
            for (auto& Instr : *CurrBB) {
                if (llvm::isa<llvm::CallBase>(&Instr)) {
                    llvm::CallBase *Call = llvm::dyn_cast<llvm::CallBase>(&Instr);
                    if (Call) {
                        llvm::Function *CalledFunc = Call->getCalledFunction();
                        if (CalledFunc) {
                            CallFunctions.push_back(CalledFunc);
                        }
                    }
                }
            }

            for (auto NextBB : llvm::successors(CurrBB))
                stackBBs.push(NextBB);

        } while (stackBBs.empty() == false);

        return CallFunctions;
    }

    void GetCallGraph(llvm::Module& Module) {
        // Get all basic blocks in the function.
        // Iterate through the instructions in the BB looking for call instrs.

        for (auto& Function : Module) {
            llvm::StringRef FuncName = Function.getName();
            auto CallInstrs = GetCallsInFunction(&Function);
            CallGraph[FuncName] = CallInstrs;
        }

        return;
    }

    void FilterDeadFunctions(const std::vector<llvm::StringRef>& FuncNames) {
        for (const auto& Name : FuncNames) {
            auto it = CallGraph.find(Name);
            if (it != CallGraph.end()) {
                const auto& FuncList = it->second;

                // Remove functions that exist in FuncList
                DeadFunctions.erase(std::remove_if(DeadFunctions.begin(), DeadFunctions.end(),
                    [&FuncList](llvm::Function* DeadFunc) {
                        return std::find(FuncList.begin(), FuncList.end(), DeadFunc) != FuncList.end();
                    }), DeadFunctions.end());
            }
        }
    }

    void GetDeadFunctions(llvm::Module& Module) {
        // Get all call references in Main function
        std::vector<llvm::Function*> Funcs = CallGraph[MainFunction->getName()];
        std::vector<llvm::StringRef> FuncNames{};
        for (auto& F : Funcs) {
            FuncNames.push_back(F->getName());
        }

        // Loop CallGraph and filter functions that aren't called from main.
        for (const auto& [key, value] : CallGraph) {
            if (std::find(FuncNames.begin(), FuncNames.end(), key) == FuncNames.end() && key != llvm::StringRef("main")) {
                auto Func = Module.getFunction(llvm::StringRef(key));
                DeadFunctions.push_back(Func);
            }
        }

        // Remove called functions in dead functions list
        FilterDeadFunctions(FuncNames);

        // Remove dead functions from program CFG
        RemoveDeadFunctionsCFG(Module);
    }

    struct DeadFunctionAnalyzer : public llvm::PassInfoMixin<DeadFunctionAnalyzer> {
        llvm::PreservedAnalyses run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM) {

            GetMainFunction(Module);
            if (!MainFunction) {
                llvm::outs() << "error: main function not found\n";
                return llvm::PreservedAnalyses::all();
            }
            
            GetCallGraph(Module);

            GetDeadFunctions(Module);
            
            // Verification:
            // Output all functions
            for (const auto& Function : Module) {
                llvm::outs() << Function.getName() << "\n";
            }

            return llvm::PreservedAnalyses::all();
        }

        static bool isRequired() {
            return true;
        }

    };

};

llvm::PassPluginLibraryInfo getPassPluginInfo() {
    const auto callback = [](llvm::PassBuilder& PB) {
        PB.registerPipelineParsingCallback([&](llvm::StringRef Name, llvm::ModulePassManager& MPM,
            llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                if (Name == "dead-func") {
                    MPM.addPass(DeadFunctionAnalyzer());
                    return true;
                }
                return false;
            });
    };

    return { LLVM_PLUGIN_API_VERSION, "DeadFunctionAnalyzer", LLVM_VERSION_STRING, callback };
}

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getPassPluginInfo();
}

// To compile this program, follow the steps otlined below.
// 1. cd ../tests/
// 2. clang++ -S -emit-llvm test01.c -o test01.ll
// 3. cd ../src/
// 4. clang++ -fPIC -shared -std=c++17 -g `llvm-config --cxxflags --ldflags --system-libs --libs core passes` -o deadfunc.so DeadFunctionAnalyzer.cpp
// 6. opt -load-pass-plugin ./deadfunc.so -passes=dead-func -disable-output ../tests/test01.ll

