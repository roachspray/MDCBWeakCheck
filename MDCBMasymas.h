#ifndef _MDCBMASYMAS_H
#define	_MDCBMASYMAS_H

class PossibleMatch {
	MulOperator *mulOp;
	UDivOperator *udivOp;
	CmpInst *cmpInst;
	Value *cmpOp;

public:
	PossibleMatch(MulOperator *m) : mulOp(m), udivOp(NULL), 
	  cmpInst(NULL), cmpOp(NULL) {}

	MulOperator *getMul() const {
		return mulOp;
	}

	void setUDiv(UDivOperator *s) {
		udivOp = s;
	}

	UDivOperator *getUDiv() const {
		return udivOp;
	}

	void setCmpOp(Value *c) {
		cmpOp = c;
	}

	Value *getCmpOp() const {
		return cmpOp;
	}

	CmpInst *getCmp() const {
		return cmpInst;
	}

};

// Does not need to be a ModulePass
struct MDCBMasymasPass : ModulePass {
private:
	Module	*mod;

public:
	static char ID;
	MDCBMasymasPass() : ModulePass(ID), mod(NULL) {}

	virtual bool runOnModule(Module &);
};
#endif
