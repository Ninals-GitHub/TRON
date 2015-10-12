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
 *	eitproc.c
 *
 *       EIT processing
 */

#include "sysdepend.h"
#include <tk/sysdef.h>

/*
 * vector information
 */
typedef struct vecinfo	VECINFO;
struct vecinfo {
	UW	vec;			/* initial vector numer */
	B	*msg;			 /* message */

        /* processing function
         *           return value  1 : adjust PC by decrementing it by one instruction worth
         *                         0 : PC needs no adjustment
	 */
	W (*func)( const VECINFO*, UW vec, UW pc, UW cpsr );
};

/* display message */
LOCAL W vf_msg( const VECINFO *vi, UW vec, UW pc, UW cpsr )
{
	B	*msg = vi->msg;
	B	opt;

	if ( msg == NULL ) return 0;

        /* if the first byte of the message is not a letter, treat it as option.
         *       opt = 0 - 037 (the code prior to ' ' )
         *       \020    PC is adjusted to the previous instruction's address
	 */
	opt = 0;
	if ( *msg < ' ' ) opt = *msg++;

	DSP_F5(S,"Exception ", D,vec, S," (", S,msg, CH,')');

	return ( opt & 020 )? 1: 0;
}

/* undefined instruction */
LOCAL W vf_undef( const VECINFO *vi, UW vec, UW pc, UW cpsr )
{
	if (cpsr & PSR_T) {
		DSP_F3(S,vi->msg, CH,' ', 04X,*((UH*)(pc - 2)));
	} else {
		DSP_F3(S,vi->msg, CH,' ', 08X,*((UW*)(pc - 4)));
	}
	return 1;
}

/* data abort */
LOCAL W vf_dabort( const VECINFO *vi, UW vec, UW pc, UW cpsr )
{
	DSP_F1(S,vi->msg);
	DSP_F4(S," ADDR: ", 08X,getCP15(6, 0), S," STAT: ", 08X,getCP15(5, 0));
	return 0;
}

/*
 * vector information table
 *       this has to be filled in the ascending order of the vector number
 */
LOCAL const VECINFO VecInfoTable[] = {
  { 0,            "\020" "Undefined SWI",	vf_msg		},
  { EIT_UNDEF,    "Undefined Instruction",	vf_undef	},
  { EIT_IABORT,   "Prefetch Abort",		vf_msg		},
  { EIT_DABORT,   "Data Abort",			vf_dabort	},
  { EIT_DABORT+1, "\020" "Undefined SWI",	vf_msg		},

  { EIT_FIQ,               "Undefined FIQ",		vf_msg  },
  { EIT_IRQ(0),            "Undefined IRQ",		vf_msg  },
  { EIT_GPIO(0),           "Undefined GPIO-INT",	vf_msg  },
  { EIT_GPIO(127)+1,"\020" "Undefined SWI",		vf_msg  },

  { N_INTVEC, NULL, vf_msg }	/* terminating mark (the last vector number + 1) */
};
#define	N_VECINFO	( sizeof(VecInfoTable) / sizeof(VECINFO) )

/*
 * EIT processing
 *       *  return value   0 : monitor should keep on running
 *                         1 : return from the interrupt handler
 */
EXPORT W procEIT( UW vec )
{
	const VECINFO	*vp;
	UW	pc, cpsr;
	W	i;

	pc = getCurPCX();
	cpsr = getCurCPSR();

        /* machine-dependent interrupt processing */
	i = procHwInt(vec);
	if ( i == 2 ) return 1; /* exit from the interrupt handler immediately */

	if ( i == 0 ) {
                /* other EIT processing */
		for ( i = 1; i < N_VECINFO; ++i ) {
			if ( vec < VecInfoTable[i].vec ) break;
		}
		vp = &VecInfoTable[i-1];
		i = (*vp->func)(vp, vec, pc, cpsr);
		if ( i > 0 ) {
                        /* PC is adjusted to the previous instruction's address */
			pc -= ( (cpsr & PSR_T) != 0 )? 2: 4;
		}
	}

	DSP_F5(S,"\nPC: ", 08X,pc, S," CPSR: ", 08X,cpsr, CH,'\n');

	return 0;
}
