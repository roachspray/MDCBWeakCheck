/*
 * typical lame arr code :D
 *
 */
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#include "MDCBMasymas.h"

bool
MDCBMasymasPass::runOnModule(Module &M)
{
	mod = &M;

	for (Function &F : M) {
			/*
			 * Weakly look for the following pattern in a function.
			 *    v1 = mul nsw A, B
			 *    v2 = udiv v1, (A or B) 
			 *    v3 = [if]cmp v2, (B or A) or [if]cmp (B or A), v2
			 * This goes by use instead of by ...
			 *
			 */
		std::map<unsigned, PossibleMatch *> matchMap;
		unsigned bcnt = 0;
		unsigned icnt = 0;
		for (BasicBlock &B : F) {
			++bcnt;
			for (Instruction &I : B) {
				++icnt;
				if (isa<MulOperator>(&I)) {
					MulOperator *mo = cast<MulOperator>(&I);
					if (!mo->hasNoSignedWrap()) {
						continue;
					}

					for (auto ui = mo->user_begin(); ui != mo->user_end(); ++ui) {
						User *u = *ui;
						if (!isa<PossiblyExactOperator>(u)) {
							continue;
						}
						PossiblyExactOperator *peo = cast<PossiblyExactOperator>(u);
						if (!isa<UDivOperator>(peo)) {
							continue;
						}
						UDivOperator *sdo = cast<UDivOperator>(peo);	
						Value *numerator = sdo->getOperand(0);
						Value *denominator  = sdo->getOperand(1);
						if (mo != numerator) {
							continue;
						}
						Value *otra = NULL;
						if (denominator == mo->getOperand(0)) {
							otra = mo->getOperand(1);
						} else if (denominator == mo->getOperand(1)) {
							otra = mo->getOperand(0);
						} else {
							continue;
						}
						for (auto uui = sdo->user_begin(); uui != sdo->user_end(); ++uui) {
							User *nu = *uui;
							if (!isa<CmpInst>(nu)) {
								continue;
							}
							CmpInst *ci = cast<CmpInst>(nu);
							if (ci->getOperand(0) == sdo && ci->getOperand(1) == otra) {
								ci->dump();
							} else if (ci->getOperand(1) == sdo && ci->getOperand(0) == otra) {
								ci->dump();
							} 
						}
					}
				}
			}
		}
	}
	return false;
}

char MDCBMasymasPass::ID = 0;
static RegisterPass<MDCBMasymasPass> X("mdcb-use-pass", "MDCB");
