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
 *	pccardio.h	system disk driver
 *
 *	PC card I/O definition (for EM1-D512)
 */

/*
 *	PC card  I/O access definition
 */
#define	InB(reg)		(UB)in_b(iob + (reg))
#define	InH(reg)		(UH)in_h(iob + (reg))
#define	OutB(reg, v)		out_b(iob + (reg), (UB)(v))
#define	OutH(reg, v)		out_h(iob + (reg), (UH)(v))
#define	CnvHIO(d)		CnvLeH(d)

#define	InForceB(iob, reg)	(UB)in_b((iob) + (reg))

/*
 *	Set the bus width for PC card I/O access
 */
#define	pcIO_8bits()		/* No Action */
#define	pcIO_16bits()		/* No Action */
