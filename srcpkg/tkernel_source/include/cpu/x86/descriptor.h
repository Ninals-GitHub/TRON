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

#ifndef	__DESCRIPTOR_H__
#define	__DESCRIPTOR_H__

#ifndef	_in_asm_source_
# include <stdint.h>
# include <tk/typedef.h>
#endif	/* _in_asm_source_ */

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
	Default Segment Descriptors
----------------------------------------------------------------------------------
*/

#define	NULL_DESCRIPTOR		0
#define	CODE_DESCRIPTOR		1
#define	DATA_DESCRIPTOR		2
#define	TSS_DESCRIPTOR		3
#define	TASK_CODE_DESCRIPTOR	4
#define	TASK_DATA_DESCRIPTOR	5
#define	MAX_NUM_DESCRIPTOR	6

#define	USER_RPL		3

#define	SEGDESC_SIZE		8

#define	SEG_KERNEL_CS		(CODE_DESCRIPTOR * SEGDESC_SIZE)
#define	SEG_KERNEL_DS		(DATA_DESCRIPTOR * SEGDESC_SIZE)
#define	SEG_USER_CS		(TASK_CODE_DESCRIPTOR * SEGDESC_SIZE + USER_RPL)
#define	SEG_USER_DS		(TASK_DATA_DESCRIPTOR * SEGDESC_SIZE + USER_RPL)
#define	SEG_TSS_SEL		(TSS_DESCRIPTOR * SEGDESC_SIZE + USER_RPL)

/*
----------------------------------------------------------------------------------
	interrupt numbers
----------------------------------------------------------------------------------
*/
#define	NUM_IDT_DESCS		256

/*
----------------------------------------------------------------------------------
	i/o bitmap
----------------------------------------------------------------------------------
*/
#define	NR_IO_MAX		0xFFFF
#define	IO_BITMAP_SIZE		(NR_IO_MAX / 8)
// last byte are always set to 1

#ifndef _in_asm_source_


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
 Funtion	:initGdt
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize global segment descriptor table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void initGdt(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_tss_esp0
 Input		:void
 Output		:void
 Return		:uint32_t*
 		 < address of esp0 of tss >
 Description	:get esp0 of tss
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT uint32_t *get_tss_esp0(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:update_tss_esp0
 Input		:uint32_t esp0
 Output		:void
 Return		:void
 Description	:update esp0 of tss
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void update_tss_esp0(uint32_t esp0)
{
	*(get_tss_esp0()) = esp0;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initKernelTss
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize tss of kernel and load it to the register
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int initKernelTss(void);

#endif	// _in_asm_source_

#endif	// __DESCRIPTOR_H__
