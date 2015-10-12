/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)syslib_depend.h (tk/EM1-D512)
 *
 *	T-Kernel/SM EM1-D512 Library
 */

#ifndef __TK_SYSLIB_DEPEND_H__
#define __TK_SYSLIB_DEPEND_H__

#include <tk/errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CPU interrupt control
 *	intsts    value of CPU CPSR register
 *	disint()  set CPSR.I = 1, A = 1, and return the original CPSR value
 *	enaint()  set CPSR.I,A = intsts.I,A. Other bits are unchanged.
 *		  return the original CPSR value.
 */
//IMPORT UINT disint( void );
//IMPORT UINT enaint( UINT intsts );
IMPORT uint32_t disint( void );
IMPORT uint32_t enaint( uint32_t intsts );
#define	DI(intsts)	( (intsts) = disint() )
#define	EI(intsts)	( enaint(intsts) )
#define	isDI(intsts)	( ((intsts) & EFLAGS_IF) == 0 )

/*
 * Interrupt vector
 *	interrupt vector is the index to the vector table.
 */
typedef UINT	INTVEC;

#define	DINTNO(intvec)	(intvec)	/* convert to interrupt definition number */

/*
 * value of interrupt vectors
 */
#define	IV_IRQ(n)	(  32 + (n) )	/* IRQ interrupt	0-95 */
#define	IV_GPIO(n)	( 128 + (n) )	/* GPIO interrupt	0-127 */

/*
 * interrupt controler
 *	intvec passed to the functions for interrupt controller must
 *      be within the valid IRQ and GPIO ranges above.
 *	If a value outside the valid range is passed, the subsequent
 *      correct behavior of the system is not guaranteed.
 */

/*
 * Set interrupt mode
 *	Set the interrupt mode specified by `intvec' to the mode given
 *  	by `mode'. If an illegal mode is given, subsequent correct behavior
 *      of the system is not guaranteed.
 *
 *	The case of IRQ
 *	mode := IM_ENA | IM_INV
 *	 or IM_DIS
 *
 *	The case of GPIO
 *	mode := IM_ENA | IM_LEVEL | (IM_HI || IM_LOW) | IM_ASYN
 *	 or  IM_ENA | IM_EDGE | (IM_HI || IM_LOW || IM_BOTH) | IM_ASYN
 *	 or  IM_DIS
 *
 *	If IM_ENA is specified, the mode setting is done, and
 *      interrupt is disabled (DisableInt) and the interrupt pin is
 *      asserted.
 *      If IM_DIS is specified, interrupt pin is disasserted.
 *      Disasserted interrupt pin doesn't generate interrupt even if
 *      it is enabled (EnabledInt).
 *      In the initial state, pins are dis-asserted (IM_DIS).
 *
 *	The initial status of the following interrupt controllers
 *	that manage GPIO interrupt is such that
 *      as if IM_ENA had been specified and interrupts are enabled
 * 	(EnableInt).
 *
 *		IRQ26	GIO6 Interrupt (GPIO port  96-111)
 *		IRQ27	GIO7 Interrupt (GPIO port 112-127)
 *		IRQ50	GIO0 Interrupt (GPIO port   0- 15)
 *		IRQ51	GIO1 Interrupt (GPIO port  16- 31)
 *		IRQ52	GIO2 Interrupt (GPIO port  32- 47)
 *		IRQ53	GIO3 Interrupt (GPIO port  48- 63)
 *		IRQ79	GIO4 Interrupt (GPIO port  64- 79)
 *		IRQ80	GIO5 Interrupt (GPIO port  80- 95)
 */
IMPORT void SetIntMode( INTVEC intvec, UINT mode );

#define	IM_ENA		0x0001	/* interrupt pin enabled */
#define	IM_DIS		0x0000	/* interrupt pin disabled */
#define	IM_INV		0x0002	/* inverted logic */
#define	IM_LEVEL	0x0200	/* level */
#define	IM_EDGE		0x0000	/* edge */
#define	IM_HI		0x0000	/* high-level/leading-edge */
#define	IM_LOW		0x0100	/* low-level/trailing-edge */
#define	IM_BOTH		0x0400	/* both edge */
#define	IM_ASYN		0x0800	/* asynchronous */

/*
 * Enable interrupt
 *	enable interrupt specified by intvec
 */
IMPORT void EnableInt( INTVEC intvec );

/*
 * Disable interrupt
 *	disable interrupt specified by intvec
 */
IMPORT void DisableInt( INTVEC intvec );

/*
 * Clear interrupt request
 *	clear the request of interrupt specified by intvec
 *	We need to clear the requests only in the case of edge-trigger interrupts.
 */
IMPORT void ClearInt( INTVEC intvec );

/*
 * Check the existence of interrupt request
 *	check the existence of request for interrupt specified by intvec
 *	If there is, TRUE (non-zero value) is returned
 *
 *	The existence of the request of interrupt is checked by
 *      'raw' status register.
 */
IMPORT BOOL CheckInt( INTVEC intvec );

/*
 * Issue EOI(End Of Interrupt)
 */
#define EndOfInt(intvec)


#ifdef __cplusplus
}
#endif
#endif /* __TK_SYSLIB_DEPEND_H__ */
