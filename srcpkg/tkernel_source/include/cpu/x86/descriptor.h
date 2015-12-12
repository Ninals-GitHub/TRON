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
#define	TLS1_DESCRIPTOR		6
#define	TLS2_DESCRIPTOR		7
#define	TLS3_DESCRIPTOR		8
#define	MAX_NUM_DESCRIPTOR	9

#define	TLS_BASE_ENTRY		TLS1_DESCRIPTOR
#define	NR_TLS_ENTRYIES		3

#define	USER_RPL		3
#define	LDT_INDEX		0x4

#define	SEGDESC_SIZE		8

#define	SEG_KERNEL_CS		(CODE_DESCRIPTOR * SEGDESC_SIZE)
#define	SEG_KERNEL_DS		(DATA_DESCRIPTOR * SEGDESC_SIZE)
#define	SEG_USER_CS		(TASK_CODE_DESCRIPTOR * SEGDESC_SIZE + USER_RPL)
#define	SEG_USER_DS		(TASK_DATA_DESCRIPTOR * SEGDESC_SIZE + USER_RPL)
#define	SEG_TSS_SEL		(TSS_DESCRIPTOR * SEGDESC_SIZE + USER_RPL)


/*
----------------------------------------------------------------------------------
	Segment Descriptor
----------------------------------------------------------------------------------
*/
/* hi definitions								*/
#define	SEGDESC_MASK_BASEADDRESS_HI	MAKE_MASK32(24, 31)
#define	SEGDESC_MASK_LIMIT_HI		MAKE_MASK32(16, 19)
#define	SEGDESC_MASK_DPL		MAKE_MASK32(13, 14)
#define	SEGDESC_MASK_TYPE_UPPER2	MAKE_MASK32(10,11)
#define	SEGDESC_MASK_TYPE		MAKE_MASK32(8,11)
#define	SEGDESC_BASEADDRESS_MID_SHIFT	16
#define	SEGDESC_MASK_BASEADDRESS_MID	MAKE_MASK32(0, 7)

#define	MASK_BASEADDRESS_HI		SEGDESC_MASK_BASEADDRESS_HI
#define	MASK_BASEADDRESS_MID		MAKE_MASK32(16, 23)
#define	BASEADDRESS_MID_SHIFT		16
#define	MASK_LIMIT_HI			SEGDESC_MASK_LIMIT_HI

#define	SEGDESC_BIT_TYPE_SHIFT		8
#define	SEGDESC_BIT_TYPE_UPPER2_SHIFT	10
#define	SEGDESC_BIT_TYPE		MAKE_BIT32(SEGDESC_BIT_TYPE_SHIFT)
#define	SEGDESC_BIT_S_SHIFT		12
#define	SEGDESC_BIT_S			MAKE_BIT32(SEGDESC_BIT_S_SHIFT)
#define	SEGDESC_BIT_DPL_SHIFT		13
#define	SEGDESC_BIT_DPL			MAKE_BIT32(SEGDESC_BIT_DPL_SHIFT)
#define	SEGDESC_BIT_P_SHIFT		15
#define	SEGDESC_BIT_P			MAKE_BIT32(SEGDESC_BIT_P_SHIFT)
#define	SEGDESC_BIT_AVL_SHIFT		20
#define	SEGDESC_BIT_AVL			MAKE_BIT32(SEGDESC_BIT_AVL_SHIFT)
#define	SEGDESC_BIT_DB_SHIFT		22
#define	SEGDESC_BIT_DB			MAKE_BIT32(SEGDESC_BIT_DB_SHIFT)
#define	SEGDESC_BIT_G_SHIFT		23
#define	SEGDESC_BIT_G			MAKE_BIT32(SEGDESC_BIT_G_SHIFT)

/* low definitions								*/
#define	SEGDESC_MASK_BASEADDRESS_LOW	MAKE_MASK32(16, 31)
#define	SEGDESC_MASK_LIMIT_LOW		MAKE_MASK32(0, 15)

#define	MASK_BASEADDRESS_LOW		MAKE_MASK32(0, 15)
#define	BASEADDRESS_LOW_SHIFT		16

#define	TLS_PRESENT			SEGDESC_BIT_P
#define	TLS_NO_READ_EXEC_ONLY		(1 << (SEGDESC_BIT_TYPE_SHIFT + 1))

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

struct segment_desc {
	uint32_t	low;
	uint32_t	hi;
};

struct user_desc;

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
IMPORT uint32_t *get_tss_esp0(void);

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

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_user_desc_to_seg_desc
 Input		:struct task *task
 		 < task to set its tls >
 		 struct user_desc *udesc
 		 < user descriptor to set >
 Output		:void
 Return		:void
 Description	:set user descriptor to task's tls
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void set_user_desc_to_seg_desc(struct task *task, struct user_desc *udesc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:update_tls_descriptor
 Input		:struct task *task
 		 < user task to update its tls descriptor in gdt >
 		 int gdt_index
 		 < index of tls segment descriptor in ddt to update >
 Output		:void
 Return		:void
 Description	:update user task's tls descriptor in gdt
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void update_tls_descriptor(struct task *task, int gdt_index);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_user_desc_from_seg_desc
 Input		:struct task *task
 		 < task to get its tls >
 		 struct user_desc *udesc
 		 < user descriptor to get >
 Output		:void
 Return		:void
 Description	:get user descriptor to task's tls
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void get_user_desc_from_seg_desc(struct task *task, struct user_desc *udesc);

#endif	// _in_asm_source_

#endif	// __DESCRIPTOR_H__
