#include "llvm/ADT/PriorityQueue.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TableGen/Error.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include <fstream>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <string.h>
#include <string>

using namespace llvm;


namespace {

struct CGRAPH : public ModulePass {

  static char ID;
  LLVMContext *Context;

  CGRAPH() : ModulePass(ID) {}

  bool doInitialization(Module &M) { return true; }

  bool doFinalization(Module &M) { return false; }

  virtual bool runOnModule(Module &M) {

    std::ofstream func_va;
    char *pathname = "./cgraph.dot";

    func_va.open(pathname, std::ios_base::out);
    func_va << "digraph \"Call graph\" \n { label=\"Call graph\";";
    func_va << "Node0"
            << "[shape=record,label=\"{START}\"];\n";

    for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
      if ((F->empty())) {
        // nothing
      } 
			else {
        errs() << "[" << F->getName() << "]: ";
        func_va << "Node0"
                << "->" << F->getName().str() << ";\n";

        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
          BasicBlock &b = *BB;
          const TerminatorInst *TInst = BB->getTerminator();

          for (BasicBlock::iterator i = b.begin(), ie = b.end(); i != ie; ++i) {

            if (CallInst *v = dyn_cast<CallInst>(&*i)) {
							// if getCalledFunction returns nullptr, that means it's indirect call
              if ((v->getCalledFunction()) == nullptr) {

                if (LoadInst *ld = dyn_cast<LoadInst>(v->getCalledValue())) {

									// Here checking for function call via struct field 
                  if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(
                          ld->getPointerOperand())) {

                    Value *sv2 = gep->getPointerOperand()->stripPointerCasts();

                    std::queue<User *> queue_user;
                    for (User *ur : sv2->users())
                      queue_user.push(ur);

                    while ((!queue_user.empty())) {
                      User *urv = queue_user.front();
                      queue_user.pop();

                      if (urv != nullptr) {
                        if (GetElementPtrInst *gep2 =
                                dyn_cast<GetElementPtrInst>(urv)) {

                          std::queue<User *> queue_user2;
                          for (User *ur2 : gep2->users())
                            queue_user2.push(ur2);
                          while ((!queue_user2.empty())) {

                            User *urv2 = queue_user2.front();

                            queue_user2.pop();
                            if (urv2 != nullptr) {

                              if (StoreInst *bt = dyn_cast<StoreInst>(urv2)) {

                                Value *sv =
                                    bt->getValueOperand()->stripPointerCasts();

                                StringRef fname = sv->getName();
                                errs() << "[" << fname << "]  ";
                                func_va << F->getName().str() << "->"
                                        << fname.str() << ";\n";
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                  std::queue<User *> queue_user;

                  for (User *ur : ld->getPointerOperand()->users())
                    queue_user.push(ur);
                  while ((!queue_user.empty())) {
                    User *urv = queue_user.front();
                    queue_user.pop();
                    if (urv != nullptr) {
                      if (StoreInst *b = dyn_cast<StoreInst>(urv)) {

                        Value *sv = b->getValueOperand()->stripPointerCasts();

                        StringRef fname = sv->getName();
                        errs() << "[" << fname << "]  ";
                        func_va << F->getName().str() << "->" << fname.str()
                                << ";\n";
                      }
                    }
                  }
                }

              } 
							else {
								// This is for direct function call
                errs() << "[" << v->getCalledFunction()->getName() << "]  ";
                func_va << F->getName().str() << "->"
                        << v->getCalledFunction()->getName().str() << ";\n";
              }
            }
          }
        }
      }
      
      errs() << "\n";
    }
    func_va << "}";

    func_va.close();

    return false;
  }

  virtual bool runOnFunction(Function &F) { return false; }
};

} 

// register pass
char CGRAPH::ID = 0;

INITIALIZE_PASS(CGRAPH, "CGRAPH", "CGRAPH", false, false);

ModulePass *llvm::createCGRAPHPass() { return new CGRAPH(); }
