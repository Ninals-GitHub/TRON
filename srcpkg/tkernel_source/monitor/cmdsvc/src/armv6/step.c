/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	step.c
 *
 *       calculate step address
 */

#include "../cmdsvc.h"

#define	aINSTSZ		4		// ARM instruction size
#define	tINSTSZ		2		// THUMB instruction size
#define REGBIT(reg)	(1 << (reg))	// register bit
#define REGSZ		4		// register size
#define	regPC		15		// PC register
#define	regSP		13		// SP register

LOCAL	UW	curCPSR;		// cpsr
LOCAL	UW	nextPC;			// the next PC value
LOCAL	W	nextLen;		// size of the next instruction(2 or 4)
LOCAL	UW	repInst;		// instruction to replace
LOCAL	W	repReg;			// register to be replaced
LOCAL	W	trcNext;		// NEXT trace mode

/*
        extraction of fields of an instruction
*/
LOCAL	UW	getInstField(UW mask, W sht)
{
	return (repInst & mask) >> sht;
}
/*
        set the register field of the instruction to be replaced
*/
LOCAL	void	setRepInst(UW mask, W sht)
{
	repInst = (repInst & ~mask) | ((repReg & 0xF) << sht);
}
/*
        Obtain an unsed register for replacement
*/
LOCAL	W	getRepReg(UW reg)
{
	W	i;

	for (i = 0; i < 16 && (reg & 0x1); i++, reg >>= 1);
	return i + 0x10;	// register number + flag
}
/*
        Validate instruction execution condition
*/
LOCAL	W	checkCond(W cond)
{
	UW	sr = curCPSR;

	switch(cond) {
	case 0:		// EQ:	z
		if (sr & PSR_Z) return 1;			break;
	case 1:		// NE:	!z
		if (!(sr & PSR_Z)) return 1;			break;
	case 2:		// CS: c
		if (sr & PSR_C) return 1;			break;
	case 3:		// CC: !c
		if (!(sr & PSR_C)) return 1;			break;
	case 4:		// MI: n
		if (sr & PSR_N) return 1;			break;
	case 5:		// PL: !n
		if (!(sr & PSR_N)) return 1;			break;
	case 6:		// VS: v
		if (sr & PSR_V) return 1;			break;
	case 7:		// VC: !v
		if (!(sr & PSR_V)) return 1;			break;
	case 8:		// HI: c && !z
		if ((sr & (PSR_C | PSR_Z)) == PSR_C) return 1;	break;
	case 9:		// LS: !c || z
		if (!(sr& PSR_C) || (sr & PSR_Z)) return 1;	break;
	case 12:	// GT: !z && (n == v)
		if (sr & PSR_Z) return 0;
	case 10:	// GE: n == v
		sr &= PSR_N | PSR_V;
		if (sr == 0 || sr == (PSR_N | PSR_V)) return 1;	break;
	case 13:	// LE: z || (n != v)
		if (sr & PSR_Z) return 1;
	case 11:	// LT: n != v
		sr &= PSR_N | PSR_V;
		if (sr == PSR_N || sr == PSR_V) return 1;	break;
	case 14:	// AL:
	case 15:	// NV:
		return 1;
	}
	return 0;
}
/*
        non-branching instruction
*/
LOCAL	void	noBranch(UW inst)
{
}
/*
        ARM: Bcond / BLX(1) instruction
*/
LOCAL	void	armBInst(UW inst)
{
	W	off;

        // BL is not handled during NEXT trace
	if (trcNext) {
		if (inst & 0x01000000) return;	// BL
		if (inst >= 0xF0000000) return;	// BLX(1)
	}
	off = (inst & 0x00FFFFFF) << 2;
	if (off & 0x02000000) off -= 0x04000000;	// sign extension
	nextPC += aINSTSZ + off;

	if (inst >= 0xF0000000) {	// BLX(1)
		nextPC += getInstField(0x01000000, 24) << 1;
                // adjust for Thumb mode (on two bytes boundary)
		nextLen = tINSTSZ;	// THUMB mode
	}
}
/*
        ARM: BX / BLX(2) instruction
*/
LOCAL	void	armBxInst(UW inst)
{
        // BL is not handled during NEXT trace
	if (trcNext) {
		if (inst & 0x00000020) return;	// BLX(2)
	}
	inst &= 0x0000000F;
	if (inst == regPC) {
		nextPC += aINSTSZ;
	} else {
		nextPC = getRegister(inst);
	}
	if (nextPC & 1) nextLen = tINSTSZ;	// THUMB mode
}
/*
        ARM: data processing instruction rd = pc
*/
LOCAL	void	armOpInst(UW inst)
{
	W	rn, rm;
	UW	usereg;

        // TST, TEQ, CMP, CMPN instructions are not handled
	rm = inst & 0x01E00000;
	if (rm >= 0x01000000 && rm <= 0x01600000) return;

        // OP1 register
	rn = getInstField(0x000F0000, 16);
	usereg = REGBIT(rn);

        // OP2 register
	if (!(inst & 0x02000000)) {
		rm = getInstField(0x0000000F, 0);
		usereg |= REGBIT(rm);
		if (inst & 0x00000010)	// shift length register
			usereg |= REGBIT(getInstField(0x00000F00, 8));
	}
        //register to be replaced
	repReg = getRepReg(usereg);

        // Dest register replacement
	setRepInst(0x0000F000, 12);

        // OP1 register replacement
	if (rn == regPC) setRepInst(0x000F0000, 16);

        // OP2 register replacement
	if (rm == regPC) setRepInst(0x0000000F, 0);

        // if S bit is set, we obtain the next mode from spsr.
	if ((inst & 0x00100000) && (getCurSPSR() & PSR_T)) nextLen = tINSTSZ;
}
/*
        ARM: LDR pc instruction
*/
LOCAL	void	armLdrInst(UW inst)
{
	W	rn, roff;
	UW	usereg;

        // base register
	rn = getInstField(0x000F0000, 16);
	usereg = REGBIT(rn);

        // OFF register
	roff = 0;
	if (inst & 0x02000000) {
		roff = getInstField(0x0000000F, 0);
		usereg |= REGBIT(roff);
	}

        //register to be replaced
	repReg = getRepReg(usereg);

        // Dest register replacement
	setRepInst(0x0000F000, 12);

        // base register replacement
	if (rn == regPC) setRepInst(0x000F0000, 16);

        // offset register replacement
	if (roff == regPC) setRepInst(0x0000000F, 0);
}
/*
        ARM: LDM {pc} instruction
*/
LOCAL	void	armLdmInst(UW inst)
{
	W	i, off;
	UW	baddr;

        // memory base address
	baddr = getRegister(getInstField(0x000F0000, 16));

        // obtain PC address offset
	off = (inst & 0x01000000) ? REGSZ : 0;	// preindex

	if (inst & 0x00800000) {	// UP
		for (i = 0; i < regPC; i++) {
			if (inst & REGBIT(i)) off += REGSZ;
		}
		baddr += off;
	} else {			// DOWN
		baddr -= off;
	}

        // Extract the value set to PC
	readMem(baddr, &nextPC, REGSZ, 2);

        // if S bit is set, we obtain the next mode from spsr.
	if ((inst & 0x00400000) && (getCurSPSR() & PSR_T)) nextLen = tINSTSZ;
}
/*
        ARM: MRS / MRC instruction rd = pc
*/
LOCAL	void	armMrcsInst(UW inst)
{
        //register to be replaced
	repReg = getRepReg(0);

        // Dest register replacement
	setRepInst(0x0000F000, 12);
}
#if CPU_ARMv6
/*
        ARM: RFE instruction
*/
LOCAL	void	armRfeInst(UW inst)
{
	W	off;
	UW	baddr, saddr, paddr, spsr;

        // memory base address
	baddr = getRegister(getInstField(0x000F0000, 16));

        // obtain PC address offset
	off = (inst & 0x01000000) ? REGSZ : 0;	// preindex

	if (inst & 0x00800000) {		// UP
		paddr = baddr + off;
		saddr = baddr + off + REGSZ;
	} else {				// DOWN
		paddr = baddr - off - REGSZ;
		saddr = baddr - off;
	}

        // Extract the value set to PC
	readMem(paddr, &nextPC, REGSZ, 2);

        // obtain the next mode from the saved spsr inside stack.
	readMem(saddr, &spsr, REGSZ, 2);
	if (spsr & PSR_T) nextLen = tINSTSZ;
}
#endif
/*
        THUMB: Bcond instruction
*/
LOCAL	void	thumbBcondInst(UW inst)
{
	W	cond, off;

	cond = getInstField(0x0F00, 8);

        // undefined instruction is not supported
	if (cond == 14) return;

        // check conditions
	if (!checkCond(cond)) return;

	off = (inst & 0x00FF) << 1;
	if (off >= 0x100) off -= 0x200;		// sign extension
	nextPC += tINSTSZ + off;
}
/*
        THUMB: B instruction
*/
LOCAL	void	thumbBInst(UW inst)
{
	W	off;

	off = (inst & 0x07FF) << 1;
	if (off >= 0x800) off -= 0x1000;	// sign extension
	nextPC += tINSTSZ + off;
}
/*
        THUMB: BX / BLX(2) instruction
*/
LOCAL	void	thumbBxInst(UW inst)
{
        // BL is not handled during NEXT trace
	if (inst & 0x0080) {	// BLX(2)
		if (trcNext) return;
	}
	inst = getInstField(0x0078, 3);
	if (inst == regPC) {
		nextPC += tINSTSZ;
	} else {
		nextPC = getRegister(inst);	//  including Hi register
	}
	if (!(nextPC & 1)) nextLen = aINSTSZ;
}
/*
        THUMB: BL / BLX(1) instruction
*/
LOCAL	void	thumbBlInst(UW inst)
{
	W	off;
	UH	inst2;

        // BL is not handled during NEXT trace
	if (trcNext) return;

        // Extract 2nd instruction
	readMem(nextPC, &inst2, tINSTSZ, 2);

	off = (inst & 0x07FF) << 12;
	if (off >= 0x400000) off -= 0x800000;	// sign extension
	nextPC += tINSTSZ + off + ((inst2 & 0x07FF) << 1);
	if (!(inst2 & 0x1000)) nextLen = aINSTSZ;	// BLX(1)
}
/*
        THUMB: ADD|CMP|MOV Rd/Rn,Rm instruction rd = pc
*/
LOCAL	void	thumbOp6Inst(UW inst)
{
	UW	op, dreg, mreg;

	op = inst & 0x0300;
	if (op == 0x0100) return;	// compare instruction

	dreg = getInstField(0x0007, 0);
	if (inst & 0x0080) dreg |= 0x8;	// Hi register
	if (dreg != regPC) return;	// not PC register

	mreg = getInstField(0x0078, 3);	// including Hi register
	nextPC = getRegister(mreg) & ~(tINSTSZ - 1);

        // calculate PC
	if (op == 0x0000) nextPC += getRegister(dreg);	// ADD
}
/*
        THUMB: POP instruction pc
*/
LOCAL	void	thumbPopInst(UW inst)
{
	W	i;
	UW	sp;

	sp = getRegister(regSP);
	for (i = 0; i < 8; i++) {
		if (inst & REGBIT(i)) sp += REGSZ;
	}
        // extract PC
	readMem(sp, &nextPC, REGSZ, 2);
}

// Arm instruction decode table
typedef	struct {
	UW	mask;			// mask
	UW	code;			// code
	void	(*calcNextPC)(UW inst);	// calculate PC
} INST_T;

LOCAL	const	INST_T	instArm[] = {
      { 0xFFFFFFFF, 0xE1A00000, noBranch},	// NOP
      { 0xFFF000F0, 0xE1200070, noBranch},	// BKPT(ARM5T)
      { 0x0FFFFFF0, 0x012FFF10, armBxInst},	// BX
      { 0xFE000000, 0xFA000000, armBInst},	// BLX(1)(ARM5T)
      { 0x0FF000F0, 0x01200030, armBxInst},	// BLX(2)(ARM5T)
      { 0x0E000000, 0x0A000000, armBInst},	// B,BL
      { 0x0F000000, 0x0F000000, noBranch},	// SWI
      { 0x0C10F000, 0x0410F000, armLdrInst},	// LDR pc
      { 0x0E108000, 0x08108000, armLdmInst},	// LDM {pc}
//    { 0x0F000010, 0x0E000000, noBranch},	// CDP
//    { 0x0E000000, 0x0C000000, noBranch},	// LDC/STC
      { 0x0F10F010, 0x0E10F010, armMrcsInst},	// MRC pc
      { 0x0FBFFFFF, 0x010FF000, armMrcsInst},	// MRS pc
//    { 0x0F0000F0, 0x00000090, noBranch},	// MULL
      { 0x0E000090, 0x00000090, noBranch},	// LDR/STR Half/SByte
//    { 0x0DB0F000, 0x0120F000, noBranch},	// MSR
//    { 0x0FB00FF0, 0x01000090, noBranch},	// SWP
      { 0x0C00F000, 0x0000F000, armOpInst},	// ADD/SUB.. rd = pc
#if CPU_ARMv6
      { 0xFE50FFFF, 0xF8100A00, armRfeInst},	// RFE(ARMv6)
#endif
      {	0 }
};

// Thumb instruction decode table
LOCAL	const	INST_T	instThumb[] = {
//    { 0xF801, 0xE800, thumbBlInst},	// BLX(1)(ARM5T) 2Nd Inst
      { 0xFF00, 0xBE00, noBranch},	// BKPT(ARM5T)
      { 0xFFFF, 0x46C0, noBranch},	// NOP
      { 0xFF00, 0xDF00, noBranch},	// SWI
      { 0xF800, 0xE000, thumbBInst},	// B <label>
      { 0xF000, 0xD000, thumbBcondInst},// B<cond> <label>
      { 0xF000, 0xF000, thumbBlInst},	// BL,BLX(1)(ARM5T)
      { 0xFF80, 0x4780, thumbBxInst},	// BLX(2)(ARM5T)
      { 0xFF00, 0x4700, thumbBxInst},	// BX
//    { 0xFC00, 0x1800, noBranch},	// OP(1)ADD|SUB Rd,Rn,Rm
//    { 0xFC00, 0x1C00, noBranch},	// OP(2)ADD|SUB Rd,Rn,#imm3
//    { 0xE000, 0x2000, noBranch},	// OP(3)<OP> Rd/Rn,#imm8
//    { 0xE000, 0x0000, noBranch},	// OP(4)LSL|LSR|ASR Rd, Rn, Rn,#shift
//    { 0xFC00, 0x4000, noBranch},	// OP(5)<OP> Rd/Rn,Rm/Rs
      { 0xFC00, 0x4400, thumbOp6Inst},	// OP(6)ADD|CMP|MOV Rd/Rn,Rm
//    { 0xF000, 0xA000, noBranch},	// OP(7)ADD Rd,SP|PC,#imm8
//    { 0xFF00, 0xB000, noBranch},	// OP(8)ADD|SUB SP,SP,#imm7
//    { 0xE000, 0x6000, noBranch},	// LS(1)LDR|STR(B) Rd,[Rn,#off5]
//    { 0xF000, 0x8000, noBranch},	// LS(2)LDRH|STRH Rd,[Rn,#off5]
//    { 0xF000, 0x5000, noBranch},	// LS(3)LDR|STR(S){H|B} Rd,[Rn,Rm]
//    { 0xF800, 0x4800, noBranch},	// LS(4)LDR Rd,[PC,#off8]
//    { 0xF000, 0x9000, noBranch},	// LS(5)LDR|STR Rd,[SP,#off8]
//    { 0xF000, 0xC000, noBranch},	// LDMIA|STMIA Rn!,{<reg list>}
      { 0xFD00, 0xBD00, thumbPopInst},	// POP {<reg list,PC}>}
      { 0 }
};
/*
        Obtain the next step address
*/
EXPORT	W	getStepAddr(UW pc, UW cpsr, W mode, UW* npc, UW *rep)
{
	W	len;
	UW	inst;
	INST_T	*tab;

        // set mode
	len = (cpsr & PSR_T) ? tINSTSZ : aINSTSZ;
	curCPSR = cpsr;		// cpsr
	repReg = 0;		// register to replace
	repInst = 0;		// instruction to replace
	nextPC = pc + len;	// the next PC value for non-branching instruction
	nextLen = len;		// size of the next instruction
	trcNext = mode;		// NEXT trace

        // extract op code
	if (readMem(pc, &inst, len, 2) != len) goto EXIT;
	repInst = inst;		// instruction to replace

	if (len == 4) {		// ARM instruction
                // check the conditional execution
		if (!checkCond(getInstField(0xF0000000, 28))) goto EXIT;
		tab = (INST_T*)instArm;
	} else {		// THUMB instruction
		tab = (INST_T*)instThumb;
		inst &= 0xFFFF;
	}

        // search instruction and calculate next PC
	for ( ; tab->mask != 0; tab++) {
		if ((inst & tab->mask) == tab->code) {
			(*(tab->calcNextPC))(inst);
			break;
		}
	}
EXIT:
	*npc = nextPC & ~(nextLen - 1);
	*rep = repInst;
	return nextLen | (repReg << 4);
}
