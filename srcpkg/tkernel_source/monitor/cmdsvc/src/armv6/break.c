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
 *	break.c
 *
 *     break/trace processing (after ARMv6)
 */

#include "../cmdsvc.h"
#include <sys/sysinfo.h>

// SW breakpoint code (BKPT instruction)
#define	BREAK_ARM	0xE1200070
#define	BREAK_THUMB	0xBE00

/*
	breakpoint data
*/
typedef struct {
	UW	addr;			// break address
	UW	code;			// saved data
	UW	atr;			// break attribute
	H	sz;			//code size (2 or 4)
	UB	cmd[L_BPCMD];		// executed command
} BRKPT;

#define	MAX_SBP		(8)		// maximum number of SW breakpoint
#define	MAX_IBP		(0)
#define	MAX_OBP		(0)
#define	MAX_BRKPT	(MAX_SBP + MAX_IBP + MAX_OBP)

LOCAL	BRKPT	brkPt[MAX_BRKPT + 1];	// breakpoint data
					// the last is temorary break

/*
        step point data
        * used for trace and temporary step processing
*/
typedef struct {
	UW	addr;			//step address
	UW	code;			// step save data
	UW	pc;			// address of replaced instruction
	UW	inst;			// replaced instruction
	UW	regval;			// replaced register value
	H	reg;			// replaced register number
	H	sz;			// code size ( 2 / 4)
} STEPPT;

LOCAL	STEPPT	stepPt;			// step point data

/*
        break attribute
*/
#define	BA_S		0x1000		//software break
#define	BA_I		0x2000		// instruction break
#define	BA_O		0x4000		//operand break
#define	BA_SET		0x8000		//software break released flag
#define	BA_PRE		0x0100		// break before execution
#define	BA_R		0x0200		// break on read
#define	BA_W		0x0400		// break on write
#define	BA_RW		0x0600		// break on read/write
#define	BA_TMP		0x0800		// temporary break

#define	MAX_BPATR	1

LOCAL	const struct {
	UB	name[4];		// attribute name
	UW	atr;			// attribute code
} brkAtr[MAX_BPATR] = {
	{"S   ", 0x00000000 | BA_S | BA_PRE},		// software break
};

/*
        trace data
*/
LOCAL	W	traceMode;		// trace mode
LOCAL	W	traceStep;		// number of trace steps
LOCAL	W	stepFlg;		// temporary step execution flag
LOCAL	union {
		UB	b[8];
		UW	w[2];		// to align on word boundary
	} sbpCode;			// SW break instructions (two)

/*
        CP14 register manipulation
*/
// no debug comprocessor
LOCAL	void	setDSCR(UW val) {return;}
LOCAL	UW	getDSCR(void) {return 0;}
LOCAL	UW	getWFAR(void) {return 0;}
LOCAL	void	setBVR(W num, UW val) {return;}
LOCAL	void	setBCR(W num, UW val) {return;}
LOCAL	UW	getBCR(W num) {return 0;}
LOCAL	void	setWVR(W num, UW val) {return;}
LOCAL	void	setWCR(W num, UW val) {return;}
LOCAL	UW	getWCR(W num) {return 0;}

/*
        check CP14 monitor debug mode
*/
LOCAL	UW	CheckCP14(void)
{
	return getDSCR() & 0x00008000;
}
/*
        set CP14 monitor debug mode
*/
LOCAL	UW	EnableCP14(void)
{
	UW	dscr;

	dscr = getDSCR();
	dscr |=  0x00008000;	// monitor debug mode on
	dscr &= ~0x00004000;	// hold debug mode off
	setDSCR(dscr);

        /* return the success/failure of setting */
	return CheckCP14();
}
/*
        reset CP14 monitor debug mode
*/
LOCAL	void	DisableCP14(void)
{
	setDSCR(getDSCR() & ~0x00008000);
	return;
}
/*
        extract break attribute
*/
EXPORT	W	getBreakAtr(UB *name)
{
	W	i;

	if (name[4] == ' ') {
		for (i = 0; i < MAX_BPATR; i++) {
			if (*((UW*)brkAtr[i].name) == *((UW*)name))
				return brkAtr[i].atr;
		}
	}
	return E_BPATR;
}
/*
        extract break attribute string (fixed length: 4 characters)
*/
LOCAL	UB	*strBreakAtr(W atr)
{
	W	i;
static	UB	str[5];

	atr &= ~BA_SET;

	for (i = 0; i < MAX_BPATR; i++) {
		if (brkAtr[i].atr == atr) {
			memcpy(str, brkAtr[i].name, 4);
			for (i = 4; str[--i] == ' '; );
			str[i + 1] = '\0';
			return str;
		}
	}
	return NULL;
}
/*
        set breakpoint
*/
EXPORT	ER	setBreak(UW addr, W atr, UB *cmd, W cmdlen)
{
	W	ibcnt, obcnt, sbcnt, sz;
	UW	code;
	BRKPT	*bp, *p;

	if (atr == 0) atr = BA_S | BA_PRE;	// default attribute

        // unaligned address (non-W alignment) is regarded as Thumb instruction
	sz = (addr & 0x03) ? 2 : 4;
	addr &= ~(sz - 1);

	if (atr & BA_TMP) {	// temporary break is used at fixed location
		bp = &brkPt[MAX_BRKPT];
	} else {
                // find an empty slot in the table
		ibcnt = obcnt = sbcnt = 0;
		for (bp = NULL, p = brkPt; p < &brkPt[MAX_BRKPT]; p++) {
			if (p->addr == 0) {if (bp == NULL) bp = p;} // empty
			else if (p->addr == addr) bp = p;	    // update
			else if (p->atr & BA_O) obcnt++;	    // WP
			else if (p->atr & BA_I) ibcnt++;	    // HW BP
			else sbcnt++;				    // SW BP
		}
                // check for the maximum value
		if (atr & BA_O) {
			if (obcnt >= MAX_OBP) return E_HBPOVR;
		} else if (atr & BA_I) {
			if (ibcnt >= MAX_IBP) return E_HBPOVR;
		} else {
			if (sbcnt >= MAX_SBP) return E_SBPOVR;
		}
	}

	if (atr & BA_S) {
                // validate PC
		// if (invalidPC(addr)) return E_BPBAD;

                //check for read and and write access rights
		if (readMem(addr, &code, sz, 2) != sz) return E_BPBAD;
		if (writeMem(addr, &sbpCode.b[sz], sz, 2) != sz) return E_BPROM;
		writeMem(addr, &code, sz, 2);
	} else {
		code = 0;
	}

        //set breakpoint
	bp->addr = addr;
	bp->atr = atr | BA_SET;
	bp->sz = sz;
	bp->code = code;
	memset(bp->cmd, 0, L_BPCMD);
	if (cmdlen > 0) memcpy(bp->cmd, cmd, cmdlen);
	return E_OK;
}
/*
        clear breakpoint
*/
EXPORT	ER	clearBreak(UW addr)
{
	BRKPT	*p;

	if (addr == 0) {	// clear all breakpoints
		memset(&brkPt[0], 0, sizeof(brkPt));
		return E_OK;
	}
	for (p = brkPt; p < &brkPt[MAX_BRKPT]; p++) {
		if (p->addr && p->addr == (addr & ~(p->sz - 1))) {
			memset(p, 0, sizeof(BRKPT));
			return E_OK;
		}
	}
	return E_BPUDF;
}
/*
        list all breakpoints
*/
EXPORT	void	dspBreak(void)
{
	BRKPT	*p;

	for (p = brkPt; p < &brkPt[MAX_BRKPT]; p++) {
		if (p->addr == 0) continue;
                // THUMB(sz == 2) is displayed using odd address
		DSP_F3(08X,(p->addr + ((p->sz & 2) >> 1)), CH,' ',
		       S,strBreakAtr(p->atr));
		if (p->cmd[0] != '\0') {
			DSP_F3(S," \"", S,p->cmd, CH,'"');
		}
		DSP_LF;
	}
}
/*
        initialize breakpoint
*/
EXPORT	void	initBreak(void)
{
        // clear all breakpoints
	memset(&brkPt[0], 0, sizeof(brkPt));

        // clear all step points
	memset(&stepPt, 0, sizeof(stepPt));

        // initialize others,
	traceMode = traceStep = stepFlg = 0;

        // SW break instruction (undefined instruction)
	*((UH*)&sbpCode.b[2]) = BREAK_THUMB;
	*((UW*)&sbpCode.b[4]) = BREAK_ARM;
}
/*
        release breakpoint temporarily (monitor entry)
*/
EXPORT	W	resetBreak(UW vec)
{
	W	i, n, bpflg;
	UW	code, pc;
	BRKPT	*p;

	pc = getCurPCX();	// break address has been adjusted
	bpflg = 0;

        /* release if monitor debug mode is used */
	if (CheckCP14()) {
                // release hardware breakpoint
		for (i = 0; i < MAX_IBP; i++) {
			setBCR(i, getBCR(i) & ~1);
		}

                // release watchpoint
		for (i = 0; i < MAX_OBP; i++) {
			setWCR(i, getWCR(i) & ~1);
		}

                // monitor debug mode is set to off later
	}

        // release steppoints
	if (stepPt.addr != 0) {
		n = stepPt.sz;
		readMem(stepPt.addr, &code, n, 2);
		if (memcmp(&code, &sbpCode.b[n], n) == 0) {
			if (pc == stepPt.addr) bpflg = 0x100;
			writeMem(stepPt.addr, &stepPt.code, n, 2);
			if (stepPt.pc > 0) {
                                // restore the changed instruction (ARM instruction only)
				writeMem(stepPt.pc, &stepPt.inst, 4, 2);
                                // restore the changed register
				pc = getRegister(stepPt.reg);
				setRegister(stepPt.reg, stepPt.regval);
			}
                        // set the NEXT real PC
			if (bpflg != 0) setCurPCX(pc);
		}
	}

        // in the case of trace/step execution, SW breakpoints have been released
	if (! (traceMode || stepFlg)) {

                // temporaly release SW/HW breakpoints (including the temporary breakpoints)
		for (p = brkPt; p <= &brkPt[MAX_BRKPT]; p++) {
			if (p->addr == 0) continue;

			if (p->atr & BA_O) {
				if (vec == EIT_DDEBUG)
					bpflg = (p - brkPt) | 0x10;
			} else if (p->atr & BA_I) {
				if (pc == p->addr)
					bpflg = (p - brkPt) | 0x10;
			} else {
				readMem(p->addr, &code, n = p->sz, 2);
				if (memcmp(&code, &sbpCode.b[n], n) == 0) {
					if (pc == p->addr)
						bpflg = (p - brkPt) | 0x10;
					writeMem(p->addr, &p->code, n, 2);
					p->atr |= BA_SET;
				} else {
					p->atr &= ~BA_SET;
				}
                                // clear temporary breakpoint
				if (p->atr & BA_TMP)
					memset(p, 0, sizeof(BRKPT));
			}
		}
	}
	return bpflg;	// is PC breakpoint?
}
/*
        setting step
*/
LOCAL	void	setStep(UW pc, W mode)
{
	W	n;
	UW	cpsr, inst;

	// ARM or THUMB
	cpsr = getCurCPSR();

        // decode instruction and obtain the next branch target
	n = getStepAddr(pc, cpsr, (mode == 2) ? 1 : 0, &stepPt.addr, &inst);

	if (n >= 0x10) {	// instruction modification
                // modify instruction (ARM instruction only)
		readMem(stepPt.pc = pc, &stepPt.inst, 4, 2);
		writeMem(pc, &inst, 4, 2);
                // restore the changed register
		stepPt.reg = (n >> 4) & 0x0F;
		stepPt.regval = getRegister(stepPt.reg);
                // Set PC witht the content of the replace register
		setRegister(stepPt.reg, pc + ((cpsr & PSR_T) ? 4 : 8));
	}
        //set break command
	stepPt.sz = (n &= 0x0F);
	readMem(stepPt.addr, &stepPt.code, n, 2);
	writeMem(stepPt.addr, &sbpCode.b[n], n, 2);
}
/*
        set breakpoint (monitor exit)
*/
EXPORT	void	setupBreak(void)
{
	W	ibcnt, obcnt;
	UW	bcr, wcr, pc;
	BRKPT	*p;

	pc = getCurPCX();

        // clear steppoint
	memset(&stepPt, 0, sizeof(stepPt));

	if (traceMode) {	// trace is executed

		setStep(pc, traceMode);		// set up step

	} else {		// normal execution

                // if an unexecuted break matches the PC value
                // temporarily set up step execution, and execute one instruction only
		if (stepFlg == 0) {
			for (p = brkPt; p <= &brkPt[MAX_BRKPT]; p++) {
				if (p->addr == pc && (p->atr & BA_PRE)) {
					setStep(pc, 0);	// set up temporary step execution
					stepFlg = 1;
					return;
				}
			}
		}

		ibcnt = obcnt = 0;

                //set breakpoint
                // - unless we turn on monitor debug mode, WCR/WVR/BCR/BVR
                //   cannot be accessed
                // - depending on hardware, monitor debug mode cannot be
                //  set to on
                // So try setting monitor debug mode on, and only if it is successful,
                //  we try to set  WCR/WVR/BCR/BVR
		for (p = brkPt; p <= &brkPt[MAX_BRKPT]; p++) {
			if (p->addr == 0) continue;

			if (p->atr & BA_O) {
				if (!EnableCP14()) continue;
				wcr = getWCR(obcnt);
				wcr &= ~0x001FC1FF;
				wcr |= ((p->atr & (BA_RW)) >> 9)  << 3;
				switch (p->atr >> 24) {		// LE only
				case 2: wcr |= 0x060 << (p->addr & 2); break;
				case 4: wcr |= 0x1e0; break;
				default: /* do nothing */ break;
				}
				setWVR(obcnt, p->addr & ~3);
				setWCR(obcnt, wcr | 7);
				obcnt++;
			} else if (p->atr & BA_I) {
				if (!EnableCP14()) continue;
				bcr = getBCR(ibcnt);
				bcr &= ~0x007FC1E7;
				bcr |= (p->addr & 2) ? 0x180 : 0x060;	// LE
				setBVR(ibcnt, p->addr & ~3);
				setBCR(ibcnt, bcr | 7);
				ibcnt++;
			} else if (p->atr & BA_SET) {
				readMem(p->addr, &p->code, p->sz, 2);
				writeMem(p->addr, &sbpCode.b[p->sz], p->sz, 2);
			}
		}

                // if hardware breakpoint is not used at all
                // monitor debug mode is turned off
		if (ibcnt == 0 && obcnt == 0) DisableCP14();
	}
	stepFlg = 0;		// clear temporary step execution flag
}
/*
        stop tracing
*/
EXPORT	void	stopTrace(void)
{
	traceMode = traceStep = 0;
}
/*
        process program execution
*/
EXPORT	ER	goTrace(W trace, UW pc, UW par)
{
	W	er;

        // set trace mode
	if ((traceMode = trace) == 0) {		// normal execution
                // set temporary breakpoint
		if (par != 0) {
			er = setBreak(par, BA_S | BA_PRE | BA_TMP, NULL, 0);
			if (er < E_OK) return er;
		}
	} else {		// trace execution
		traceStep = par;
	}
	setCurPC(pc);		// set execution start address
	return E_OK;
}
/*
        prcess break exception

        return 0: continue, 1: command execution (cmd : initial command line)
*/
EXPORT	W	procBreak(W bpflg, UB **cmd)
{
	B	*mes;
	BRKPT	*bp;
	UW	pc, npc, wfar;

	bp = NULL;

	if (traceMode) {	// trace execution
                //PC holds the next PC value (by resetBreak())
		pc = getCurPC();

                // disassembly display (next instruction)
		disAssemble(&pc, &npc, wrkBuf);
		DSP_F4(08X,pc, S,": ", S,wrkBuf, CH,'\n');

		if (-- traceStep > 0) return 0;		// continue
		stopTrace();	// stop tracing

	} else {		// breakpoint
                // During temporary step execution, then do nothing and continue
		if (stepFlg) return 0;

		pc = getCurPCX();	// break address has been adjusted

                // this is not a breakpoint set by b command
		if ((bpflg & 0xF0) == 0) {
			DSP_F3(S,"Unknown break at H'", 08X,pc, CH,'\n');
			*cmd = NULL;
			return 1;
		}

		bp = &brkPt[bpflg & 0xF];
		switch (bp->atr & (BA_S | BA_I | BA_O | BA_R | BA_W)) {
		  case BA_S:		mes = "S";	break;
		  case BA_I:		mes = "E";	break;
		  case BA_O|BA_R:	mes = "R";	break;
		  case BA_O|BA_W:	mes = "W";	break;
		  case BA_O|BA_R|BA_W:	mes = "RW";	break;
		  default:		mes = "?";	break;
		}

		if ((bp->atr & BA_O) && CheckCP14()) {
                        // the address of instruction that generated operand break
                        // is fetched from WFAR
			wfar = getWFAR();
			wfar -= (getCurCPSR() & PSR_T) ? 4 : 8;
			DSP_F4(S,"Break (", S,mes, S,") at ", 08X,wfar);
			DSP_F3(S,"  (R15/PC:", 08X,pc, S,")\n");
		} else {
			DSP_F5(S,"Break (", S,mes, S,") at ", 08X,pc, CH,'\n');
		}
	}

        // restore stopped instruction
	*cmd = (bp && bp->cmd[0] != 0) ? bp->cmd : NULL;
	return 1;	// wait for command
}
