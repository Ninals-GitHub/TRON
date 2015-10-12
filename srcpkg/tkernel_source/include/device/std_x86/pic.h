/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#ifndef	__PIC_H__
#define	__PIC_H__

#include <stdint.h>
#include <device/port.h>
#include <cpu/x86/cpu_insn.h>
#include <typedef.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	For reqEnableIrq and reqDisableIrq request interrupt number
----------------------------------------------------------------------------------
*/
#define	PIC_INT_IRQ_ALL		0xFF	/* see interrup.h abou other number	*/

/*
----------------------------------------------------------------------------------
	For reqEnableIrq and reqDisableIrq return flags
----------------------------------------------------------------------------------
*/
#define	PIC_FLAGS_IRQ0		0x0001
#define	PIC_FLAGS_IRQ1		0x0002
#define	PIC_FLAGS_IRQ2		0x0004
#define	PIC_FLAGS_IRQ3		0x0008
#define	PIC_FLAGS_IRQ4		0x0010
#define	PIC_FLAGS_IRQ5		0x0020
#define	PIC_FLAGS_IRQ6		0x0040
#define	PIC_FLAGS_IRQ7		0x0080
#define	PIC_FLAGS_IRQ8		0x0100
#define	PIC_FLAGS_IRQ9		0x0200
#define	PIC_FLAGS_IRQ10		0x0400
#define	PIC_FLAGS_IRQ11		0x0800
#define	PIC_FLAGS_IRQ12		0x1000
#define	PIC_FLAGS_IRQ13		0x2000
#define	PIC_FLAGS_IRQ14		0x4000
#define	PIC_FLAGS_IRQ15		0x8000

/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initPic
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize programmable intterupt controlers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void initPic(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:reqEnableIrq
 Input		:uint8_t irq_num
 		 < irq number to enable >
 Output		:void
 Return		:uint16_t
 		 < old flags : low byte is from master, high byte is from slave >
 Description	:request to enable specified irq
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint16_t reqEnableIrq(uint8_t irq_num);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:reqDisableIrq
 Input		:uint8_t irq_num
 		 < irq number to enable >
 Output		:void
 Return		:uint16_t
 		 < old flags : low byte is from master, high byte is from slave >
 Description	:request to unenable specified irq
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint8_t reqDisableIrq(uint8_t irq_num);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:EndOfIntterrupt1
 Input		:void
 Output		:void
 Return		:void
 Description	:send eoi to only master for use of irq 0 to 7
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void EndOfInterrupt1(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:EndOfIntterrupt2
 Input		:void
 Output		:void
 Return		:void
 Description	:send eoi to master and slave for use of irq 8 to 15
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void EndOfInterrupt2(void);

#endif	// __PIC_H__
