#include "pass.hpp"

#include <map>
#include <set>
#include <vector>
#include <iostream>

using namespace llvm;

namespace UVPass {

    PreservedAnalyses UninitializedVariablePass::run(Module& Module, ModuleAnalysisManager& MAM) {
        // https://github.com/isuckatcs/how-to-compile-your-language/blob/main/src/sema.cpp#L64
        enum class State { Bottom, Unassigned, Assigned, Top };

        using Lattice = std::map<StringRef, State>;

        auto MergeStates = [](State s1, State s2) {
            if (s1 == s2)
                return s1;

            if (s1 == State::Bottom)
                return s2;

            if (s2 == State::Bottom)
                return s1;

            return State::Top;
        };

        std::set<StringRef> PendingErrors{};

        for (auto& Function : Module) {
            PendingErrors.clear();

            outs() << "Function: " << Function.getName() << "\n";
            int VarIdx = 0;

            std::vector<Lattice> CurrLattices(Function.size());

            for (auto& BB : Function) {
                Lattice tmp{};

                for (auto preds : predecessors(&BB)) {
                    for (auto&& [sref, state] : CurrLattices[preds->getNumber()]) {
                        tmp[sref] = MergeStates(tmp[sref], state);
                    }
                }

                for (auto& Inst : BB) {
                    if (isa<AllocaInst>(&Inst)) {
                        AllocaInst* AllocaInstr = dyn_cast<AllocaInst>(&Inst);
                        auto DestVal = AllocaInstr;
                        auto DestType = DestVal->getAllocatedType();
                        if (DestVal->getName().empty()) {
                            DestVal->setName("lvar_" + std::to_string(VarIdx++));
                        }
                        tmp[DestVal->getName()] = State::Unassigned;
                        continue;
                    }

                    if (isa<StoreInst>(&Inst)) {
                        StoreInst* StoreInstr = dyn_cast<StoreInst>(&Inst);
                        auto ValueOperand = StoreInstr->getValueOperand();
                        auto PtrOperand = StoreInstr->getPointerOperand();
                        
                        if (isa<Constant>(ValueOperand) || isa<ConstantFP>(ValueOperand) || isa<GlobalVariable>(ValueOperand) || 
                            isa<Argument>(ValueOperand) || isa<ConstantPointerNull>(ValueOperand) ||
                            // Double, char*, char
                            ValueOperand->getType()->isDoubleTy() || ValueOperand->getType()->isPointerTy() || ValueOperand->getType()->isIntegerTy(8)) {
                            if (tmp.find(PtrOperand->getName()) != tmp.end()) {
                                tmp[PtrOperand->getName()] = State::Assigned;
                            }
                            continue;
                        }
                    }

                    if (isa<LoadInst>(&Inst)) {
                        LoadInst* LoadInstr = dyn_cast<LoadInst>(&Inst);
                        auto* PtrOperand = LoadInstr->getPointerOperand();
                        if (tmp.find(PtrOperand->getName()) != tmp.end()) {
                            if (tmp[PtrOperand->getName()] != State::Assigned) {
                                PendingErrors.insert(PtrOperand->getName());
                            }
                        }
                        continue;
                    }

                    if (isa<GetElementPtrInst>(&Inst)) {
                        GetElementPtrInst* ElementInst = dyn_cast<GetElementPtrInst>(&Inst);
                    }

                    if (isa<ICmpInst>(&Inst) || isa<FCmpInst>(&Inst)) {
                        FCmpInst* CmpInst = dyn_cast<FCmpInst>(&Inst);
                    }

                    if (isa<CallInst>(&Inst)) {
                        CallInst* CallInstr = dyn_cast<CallInst>(&Inst);
                    }

                    if (isa<SelectInst>(&Inst)) {
                        /*
                        *   llvm::SelectInst is the LLVM IR representation of a select instruction, 
                        *   which acts like a ternary conditional operator in C/C++: cond ? val1 : val2
                        *   %result = select i1 %cond, i32 %val1, i32 %val2
                        */
                        SelectInst* SelectInstr = dyn_cast<SelectInst>(&Inst);
                    }

                    if (isa<ExtractElementInst>(&Inst) || isa<InsertElementInst>(&Inst)) {}

                    if (isa<ExtractValueInst>(&Inst) || isa<InsertValueInst>(&Inst)) {}

                    if (isa<PHINode>(&Inst)) {
                        PHINode* PhiNode = dyn_cast<PHINode>(&Inst);
                    }

                    if (isa<ReturnInst>(&Inst)) {
                        ReturnInst* ReturnInstr = dyn_cast<ReturnInst>(&Inst);
                    }

                    if (isa<BranchInst>(&Inst)) {
                        BranchInst* BrInst = dyn_cast<BranchInst>(&Inst);
                    }

                    if (isa<SwitchInst>(&Inst)) {
                        SwitchInst* SwitchInstr = dyn_cast<SwitchInst>(&Inst);
                    }

                    if (isa<IndirectBrInst>(&Inst)) {
                        IndirectBrInst* BrInst = dyn_cast<IndirectBrInst>(&Inst);
                    }

                    if (isa<CallBrInst>(&Inst)) {
                        CallBrInst* BrInst = dyn_cast<CallBrInst>(&Inst);
                    }

                    if (isa<ZExtInst>(&Inst) || isa<SExtInst>(&Inst)) {
                        SExtInst* ExtInst = dyn_cast<SExtInst>(&Inst);
                    }

                    if (isa<UIToFPInst>(&Inst) || isa<SIToFPInst>(&Inst)) {
                        SIToFPInst* CastedInt = dyn_cast<SIToFPInst>(&Inst);
                        auto PtrOperand = CastedInt->getOperand(0);
                    }

                    if (isa<FPToUIInst>(&Inst) || isa<FPToSIInst>(&Inst)) {
                        FPToSIInst* CastedInt = dyn_cast<FPToSIInst>(&Inst);
                    }

                    if (isa<IntToPtrInst>(&Inst)) {
                        IntToPtrInst* PtrInst = dyn_cast<IntToPtrInst>(&Inst);
                    }

                    if (isa<PtrToIntInst>(&Inst)) {
                        PtrToIntInst* IntInst = dyn_cast<PtrToIntInst>(&Inst);
                    }
                }

                if (CurrLattices[BB.getNumber()] != tmp) {
                    CurrLattices[BB.getNumber()] = tmp;
                }
            }

            if (!PendingErrors.empty()) {
                outs() << "Referenced unitialized values: \n";
                for (auto Error : PendingErrors) {
                    outs() << "\t" << Error << "\n";
                }
                outs() << "\n";
            }
            outs() << "---------------------\n";
        }
        
        return llvm::PreservedAnalyses::all();
    }

};   // namespace Pass



