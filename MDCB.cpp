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

#include "MDCB.h"

bool
MDCBPass::runOnModule(Module &M)
{
	mod = &M;

	for (Function &F : M) {
		unsigned bcnt = 0;
		for (BasicBlock &B : F) {
			++bcnt;

			/*
			 * Weakly look for the following pattern in a single BB.
			 *    v1 = mul nsw A, B
			 *    v2 = udiv v1, (A or B) 
			 *    v3 = [if]cmp v2, (B or A) or [if]cmp (B or A), v2
			 * This goes instruction by instruction instead of by uses...
			 *
			 */
			std::map<unsigned, PossibleMatch *> matchMap;
			unsigned icnt = 0;
			for (Instruction &I : B) {
				++icnt;
				if (isa<MulOperator>(&I)) {
					MulOperator *mo = cast<MulOperator>(&I);
					// XXX arr: Hrm.
					if (mo->hasNoSignedWrap()) {
						matchMap[icnt] = new PossibleMatch(mo);
					}
					continue;
				}

				if (isa<PossiblyExactOperator>(&I)) {
					PossiblyExactOperator *peo = cast<PossiblyExactOperator>(&I);
					if (!isa<UDivOperator>(peo)) {
						continue;
					}

					UDivOperator *sdo = cast<UDivOperator>(peo);	
					Value *numerator = sdo->getOperand(0);
					Value *denominator  = sdo->getOperand(1);

					for (auto mit = matchMap.begin(); mit != matchMap.end(); ++mit) {
						unsigned ic = mit->first;
						PossibleMatch *p = mit->second;
						MulOperator *mo = p->getMul();
						Value *valMul = mo;
						/*
						 * Want the case: udiv (mul nsw A, B) ...
						 */
						if (numerator != valMul) {
							continue;
						}
						Value *moOp1 = mo->getOperand(0);
						Value *moOp2 = mo->getOperand(1);	
						if (denominator == moOp1) {
							matchMap[ic]->setUDiv(sdo);
							p->setCmpOp(moOp2);
						} else if (denominator == moOp2) {
							matchMap[ic]->setUDiv(sdo);
							p->setCmpOp(moOp1);
						}
				
					}
					continue;
				}

				if (isa<CmpInst>(&I)) {
					Value *cOp1 = I.getOperand(0);
					Value *cOp2 = I.getOperand(1);
					for (auto mit = matchMap.begin(); mit != matchMap.end(); ++mit) {
						unsigned ic = mit->first;
						PossibleMatch *p = mit->second;
						UDivOperator *ud = p->getUDiv();
						Value *vud = ud;
						Value *pmCmpOp = p->getCmpOp();

						if (cOp1 == vud && cOp2 == pmCmpOp) {
							errs() << "Function: " << F.getName().str() <<  " block " \
							  << std::to_string(bcnt) << "\n";
						} else if (cOp1 == pmCmpOp && cOp2 == vud) {
							errs() << "Function: " << F.getName().str() <<  " block " \
							  << std::to_string(bcnt) << "\n";
						}
					}	
				}
			}

			for (auto mit = matchMap.begin(); mit != matchMap.end(); ++mit) {
				PossibleMatch *p = mit->second;
				delete p;
			}
		}
	}
	return false;
}

char MDCBPass::ID = 0;
static RegisterPass<MDCBPass> X("mdcb-pass", "MDCB");
