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
 *	register.c
 *
 *       Register-related operations (after ARMV6)
 */

#include "../cmdsvc.h"
#include <sys/sysinfo.h>

EXPORT	UW	regStack[39 + 10 + 2];

/*
        register definition table

        * registers are saved to regStack on entry to the monitor.
           The value in register ID follows the order saved in register stack (below).
           (See eitent.S)

	regStack[0-7]		r0 .. r7
		[8,9]		Entry cspr, ip
		[10]		return cspr
		[11]		return r15(pc)
		[12-18]	   USR:	r8 ..r12, sp, lr
		[19-26]	   FIQ:	spsr, r8 ..r12, sp, lr
		[27-29]	   IRQ:	spsr, sp, lr
		[30-32]	   ABT:	spsr, sp, lr
		[33-35]	   UND:	spsr, sp, lr
		[36-38]	   SVC:	spsr, sp, lr

		[39]       CP15: SCTLR             (CP15.c1.0.c0.0)
		[40-42]          TTBR0,TTBR1,TTBCR (CP15.c2.0.c0.0 - 2)
		[43]             DACR              (CP15.c3.0.c0.0)
		[44-45]          DFSR,IFSR         (CP15.c5.0.c0.0 - 1)
		[46-47]          DFAR,IFAR         (CP15.c6.0.c0.0,2)
		[48]             CTXIDR            (CP15.c13.0.c0.1)
*/

#define	L_REGNM		8
typedef	struct {
	UB	name[L_REGNM];		// register name
	UW	id;			// register ID
} REGTAB;

#define	R_GEN		0x001000	// general register
#define	R_CTL		0x002000	// control register
#define	R_GRP		0x010000	// register group

#define	R_LF		0x080000	// forced linefeed
#define	R_GAP		0x040000	// empty line

#define	R_ONLY		0x100		// disable setup
#define	SPEC(n)		(0x200 | (n))	// special

#define	ixCPSR		10		// CPSR index
#define	ixPC		11		// PC index

#define	ixUSR		12
#define	ixFIQ		(19 + 1)	// FIQ: SPSR,R8-R14
#define	ixIRQ		(27 - 4)	// IRQ: SPSR,R13,R14
#define	ixABT		(30 - 4)	// ABT: SPSR,R13,R14
#define	ixUND		(33 - 4)	// UND: SPSR,R13,R14
#define	ixSVC		(36 - 4)	// SVC: SPSR,R13,R14
#define	ixSP_SVC	(ixSVC + 5)	// SVC SP index

#define	ixCP15		39		// CP15 index
#define	ixCP15R1	(ixCP15 + 0)

#define	N_ACTREGS	(16 + 7 + 7 + 8 + 7 + 10)
#define	N_REGS		(N_ACTREGS + 3)

LOCAL	const	REGTAB	regTab[N_REGS] = {
	{"R0      ",	R_GEN + 0x00			},	// 0
	{"R1      ",	R_GEN + 0x01			},	// 1
	{"R2      ",	R_GEN + 0x02			},	// 2
	{"R3      ",	R_GEN + 0x03 + R_LF		},	// 3
	{"R4      ",	R_GEN + 0x04			},	// 4
	{"R5      ",	R_GEN + 0x05			},	// 5
	{"R6      ",	R_GEN + 0x06			},	// 6
	{"R7      ",	R_GEN + 0x07 + R_LF		},	// 7
	{"R8      ",	R_GEN + SPEC(0x00)		},	// 8
	{"R9      ",	R_GEN + SPEC(0x01)		},	// 9
	{"R10/SL  ",	R_GEN + SPEC(0x02)		},	// 10
	{"R11/FP  ",	R_GEN + SPEC(0x03) + R_LF	},	// 11
	{"R12/IP  ",	R_GEN + SPEC(0x04)		},	// 12
	{"R13/SP  ",	R_GEN + SPEC(0x05)		},	// 13
	{"R14/LR  ",	R_GEN + SPEC(0x06)		},	// 14
	{"R15/PC  ",	R_GEN + ixPC + R_LF		},	// 15

	{"R8_USR  ",	R_GEN + ixUSR + 0 + R_GAP	},	// 16
	{"R9_USR  ",	R_GEN + ixUSR + 1		},	// 17
	{"R10_USR ",	R_GEN + ixUSR + 2		},	// 18
	{"R11_USR ",	R_GEN + ixUSR + 3 + R_LF	},	// 19
	{"R12_USR ",	R_GEN + ixUSR + 4		},	// 20
	{"R13_USR ",	R_GEN + ixUSR + 5		},	// 21
	{"R14_USR ",	R_GEN + ixUSR + 6 + R_LF	},	// 22

	{"R8_FIQ  ",	R_GEN + ixFIQ + 0		},	// 23
	{"R9_FIQ  ",	R_GEN + ixFIQ + 1		},	// 24
	{"R10_FIQ ",	R_GEN + ixFIQ + 2		},	// 25
	{"R11_FIQ ",	R_GEN + ixFIQ + 3 + R_LF	},	// 26
	{"R12_FIQ ",	R_GEN + ixFIQ + 4		},	// 27
	{"R13_FIQ ",	R_GEN + ixFIQ + 5		},	// 28
	{"R14_FIQ ",	R_GEN + ixFIQ + 6 + R_LF	},	// 29

	{"R13_IRQ ",	R_GEN + ixIRQ + 5		},	// 30
	{"R14_IRQ ",	R_GEN + ixIRQ + 6		},	// 31
	{"R13_SVC ",	R_GEN + ixSVC + 5		},	// 32
	{"R14_SVC ",	R_GEN + ixSVC + 6 + R_LF	},	// 33
	{"R13_ABT ",	R_GEN + ixABT + 5		},	// 34
	{"R14_ABT ",	R_GEN + ixABT + 6		},	// 35
	{"R13_UND ",	R_GEN + ixUND + 5		},	// 36
	{"R14_UND ",	R_GEN + ixUND + 6 + R_LF	},	// 37

	{"CPSR    ",	R_CTL + ixCPSR + R_GAP		},	// 38
	{"SPSR    ",	R_CTL + SPEC(0x08)		},	// 39
	{"SPSR_FIQ",	R_CTL + ixFIQ - 1		},	// 40
	{"SPSR_IRQ",	R_CTL + ixIRQ + 4 + R_LF	},	// 41
	{"SPSR_SVC",	R_CTL + ixSVC + 4		},	// 42
	{"SPSR_ABT",	R_CTL + ixABT + 4		},	// 43
	{"SPSR_UND",	R_CTL + ixUND + 4 + R_LF	},	// 44

	{"SCTLR   ",	R_CTL + SPEC(0x0F) +  0 + R_GAP	},	// 45
	{"TTBR0   ",	R_CTL + ixCP15 +  1 + R_ONLY	},	// 46
	{"TTBR1   ",	R_CTL + ixCP15 +  2 + R_ONLY	},	// 47
	{"TTBCR   ",	R_CTL + ixCP15 +  3 + R_ONLY + R_LF},	// 48
	{"DACR    ",	R_CTL + ixCP15 +  4		},	// 49
	{"DFSR    ",	R_CTL + ixCP15 +  5		},	// 50
	{"IFSR    ",	R_CTL + ixCP15 +  6		},	// 51
	{"DFAR    ",	R_CTL + ixCP15 +  7 + R_LF	},	// 52
	{"IFAR    ",	R_CTL + ixCP15 +  8		},	// 53
	{"CTXIDR  ",	R_CTL + ixCP15 +  9 + R_LF	},	// 54

	{"G       ",	R_GRP|R_GEN			},
	{"C       ",	R_GRP|R_CTL			},
	{"A       ",	R_GRP|R_GEN|R_CTL		},
};
/*
        Searching register name
*/
EXPORT	W	searchRegister(UB *name, W grp)
{
	W	i, n, a;
	UB	bf[L_REGNM];
	REGTAB	*p;

	if (name[L_REGNM] != ' ') return -1;

	for (p = (REGTAB*)regTab, i = 0; i < N_REGS; p++, i++) {
		for (n = 0; n < L_REGNM; n++) if (p->name[n] == '/') break;
		if (n == L_REGNM) {	// no separator '/' -> a single register name
			if (memcmp(name, p->name, L_REGNM)) continue;
		} else {		// has alias
                        // check the name(s) after the separator
			memset(bf, ' ', sizeof(bf));
			memcpy(bf, p->name + (n + 1), L_REGNM - (n + 1));
			a = memcmp(name, bf, L_REGNM - n);

                        // check the name before the separtor
			memset(bf, ' ', sizeof(bf));
			memcpy(bf, p->name, n);
			if (a && memcmp(name, bf, n + 1)) continue;
		}
		if (grp == 0 && (p->id & R_GRP)) break;
		return i;
	}
	return -1;
}
/*
        obtain CPU mode index
*/
LOCAL	W	ixCpuMode(void)
{
        // obtain mode
	switch(regStack[ixCPSR] & PSR_M(31)) {
	case PSR_USR:
	case PSR_SYS:	return ixUSR;
	case PSR_FIQ:	return ixFIQ;
	case PSR_IRQ:	return ixIRQ;
	case PSR_SVC:	return ixSVC;
	case PSR_ABT:	return ixABT;
	case PSR_UND:	return ixUND;
	}
	return 0;
}
/*
        obtain register value
*/
EXPORT	UW	getRegister(W regno)
{
	W	i, ix;

	i = regTab[regno].id & (R_GRP | 0x3ff);

        // normal register
	if (i < SPEC(0)) return regStack[i & 0xff];

        // obtain mode
	ix = ixCpuMode();

        // special register
	switch(i) {
	case SPEC(0x00):	// R8
	case SPEC(0x01):	// R9
	case SPEC(0x02):	// R10
	case SPEC(0x03):	// R11
	case SPEC(0x04):	// R12
		if (ix != ixFIQ) ix = ixUSR;
	case SPEC(0x05):	// R13
	case SPEC(0x06):	// R14
		return regStack[ix + i - SPEC(0)];
	case SPEC(0x08):	// SPSR
		if (ix == ixUSR) return 0;	// undefined
		if (ix == ixFIQ) ix -= 5;
		return regStack[ix + 4];
	case SPEC(0x0F):	// CP15 R1
		return regStack[ixCP15R1];
	}
        // retur 0 on error
	return 0;
}
/*
        Set register value
*/
EXPORT	ER	setRegister(W regno, UW val)
{
	W	i, ix;

	i = regTab[regno].id & (R_GRP | 0x3ff);
	if (i & R_ONLY) return E_RONLY;		// cannot be set

	if (i < SPEC(0)) {	// normal register
		regStack[i & 0xff] = val;
		return 0;
	}

        // obtain mode
	ix = ixCpuMode();

        // special register
	switch(i) {
	case SPEC(0x00):	// R8
	case SPEC(0x01):	// R9
	case SPEC(0x02):	// R10
	case SPEC(0x03):	// R11
	case SPEC(0x04):	// R12
		if (ix != ixFIQ) ix = ixUSR;
	case SPEC(0x05):	// R13
	case SPEC(0x06):	// R14
		regStack[ix + i - SPEC(0x00)] = val;
		break;
	case SPEC(0x08):	// SPSR
		if (ix == ixUSR) break;	// undefined
		if (ix == ixFIQ) ix -= 5;
		regStack[ix + 4] = val;
		break;
	case SPEC(0x0F):	// CP15 R1
		regStack[ixCP15R1] &= MASK_CACHEMMU;
		regStack[ixCP15R1] |= val & VALID_CACHEMMU;
		break;
	default:
		return E_PAR;
	}
	return 0;
}
/*
        List the values of register (group)
                regno < 0 : default group (not specified)
*/
EXPORT	void	dispRegister(W regno)
{
	W	i, j, n, id, rid;

	if (regno >= N_REGS) return;

	id = (regno < 0) ? (R_GRP | R_GEN) : regTab[regno].id;

	for (n = i = 0; i < N_ACTREGS; i++) {
		rid = regTab[i].id;
		if (!(i == regno || ((id & R_GRP) && (rid & id)))) continue;
		if (n != 0 && (rid & R_GAP)) DSP_LF;
		if (n++ & 0x0f) DSP_S("  ");
		for (j = 0; j < L_REGNM; j++) DSP_CH(regTab[i].name[j]);
		DSP_F2(S,": ", 08X,getRegister(i));
		if (rid & R_LF) {DSP_LF; n = 0x10;}
		if ((id & R_GRP) == 0) break;
	}
	if (n & 0x0f) DSP_LF;
}
/*
        obtain CPSR register value
*/
EXPORT	UW	getCurCPSR(void)
{
	return regStack[ixCPSR];
}
/*
        obtain SPSR register value
*/
EXPORT	UW	getCurSPSR(void)
{
	return getRegister(39);
}
/*
        obtain PC register value
*/
EXPORT	UW	getCurPC(void)
{
        // set LSB = 1 for Thumb mode.
	return regStack[ixPC] | ((regStack[ixCPSR] & PSR_T) ? 1 : 0);
}
EXPORT	UW	getCurPCX(void)
{
	return regStack[ixPC];
}
EXPORT	UW	getCP15(W reg, W opcd)
{
	W	i, d;
	LOCAL	const UH reg_op[] = {
		0x1000, 0x2000, 0x2001, 0x2002, 0x3000, 0x5000, 0x5001, 0x6000,
		0x6002, 0xd001,
	};

	d = ((reg & 0x0f) << 12) | (opcd & 0x0fff);

	for (i = 0; i < sizeof(reg_op) / sizeof(UH); i++) {
		if (reg_op[i] == d) return regStack[ixCP15 + i];
	}
	return 0;
}
/*
        Set PC register value
*/
EXPORT	void	setCurPC(UW val)
{
	if (regStack[ixPC] != val) {
                // Thumb Bit is changed according to the LSB value of PC.
		if (val & 0x3)	regStack[ixCPSR] |= PSR_T;
		else		regStack[ixCPSR] &= ~PSR_T;
		regStack[ixPC] = val & ~0x1;
	}
}
EXPORT	void	setCurPCX(UW val)
{
        // Thumb Bit is not changed.
	regStack[ixPC] = val & ~0x1;
}
/*
        Set registers for BOOT
*/
EXPORT	void	setUpBoot( void *start, BootInfo *bootinfo )
{
	bootFlag = 1; /* suppress the setting register R0 upon exit of the monitor */

	regStack[ixCPSR]   = PSR_I | PSR_F | PSR_SVC;
	regStack[0]        = (UW)bootinfo;		// R0 boot parameter
	regStack[ixPC]     = (UW)start;			// PC start address
	regStack[ixSP_SVC] = (UW)&__stack_bottom;	// SP monitor stack

        // MMU enabled, Cache / Write Buffer not enabled
	regStack[ixCP15R1] &= MASK_CACHEMMU;
	regStack[ixCP15R1] |= ENB_MMUONLY;

        // system initialization processing
	resetSystem(1);
}
/*
        Check whether we can use KILL command
*/
EXPORT	W	isKillValid(void)
{
        // Has TRAP for KILL been define?
	if ( SCArea->intvec[SWI_KILLPROC] == NULL ) return -1;
	return 0;
}

#if REF_TKOBJECT
/*
        Check whether T-Kernel/DS functions can be executed?
*/
EXPORT	W	isTKDSValid(void)
{
        // Has TRAP for T-Kernel/DS been defined?
	if ( SCArea->intvec[SWI_DEBUG] == NULL ) return -1;
	return 0;
}

/*
        Display register of tasks
*/
EXPORT W PrintTaskRegister( int (*prfn)( const char *format, ... ),
				T_REGS *gr, T_EIT *er, T_CREGS *cr )
{
/*
 *	PC: 12345678             CPSR:12345678 TMF:12345678
 *	R0: 12345678 R1: 12345678 R2: 12345678 R3: 12345678
 *	R4: 12345678 R5: 12345678 R6: 12345678 R7: 12345678
 *	R8: 12345678 R9: 12345678 R10:12345678 R11:12345678
 *	IP: 12345678 LR: 12345678
 *	USP:12345678 SSP:12345678 LSID:1234   UATB:12345678
 */
	(*prfn)("PC: %08x             CPSR:%08x TMF:%08x\n",
		(UW)er->pc, er->cpsr, er->taskmode);
	(*prfn)("R0: %08x R1: %08x R2: %08x R3: %08x\n",
		gr->r[0], gr->r[1], gr->r[2], gr->r[3]);
	(*prfn)("R4: %08x R5: %08x R6: %08x R7: %08x\n",
		gr->r[4], gr->r[5], gr->r[6], gr->r[7]);
	(*prfn)("R8: %08x R9: %08x R10:%08x R11:%08x\n",
		gr->r[8], gr->r[9], gr->r[10], gr->r[11]);
	(*prfn)("IP: %08x LR: %08x\n",
		gr->r[12], (UW)gr->lr);
	(*prfn)("USP:%08x SSP:%08x LSID:%-4d   UATB:%08x\n",
		(UW)cr->usp, (UW)cr->ssp, cr->lsid, (UW)cr->uatb);
	return 6;  /* number of display lines */
}
#endif	/* REF_TKOBJECT */
