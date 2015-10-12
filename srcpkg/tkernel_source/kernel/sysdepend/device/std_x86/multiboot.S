/*
 *--------------------------------------------------------------------------------
 *    T-Kernel 2.0 x86 Extended
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the T-License 2.0.
 *--------------------------------------------------------------------------------
 *
 *--------------------------------------------------------------------------------
 */

#include <sys/sysdepend/std_x86/multiboot_def.h>

/* kernel entry */
.extern	_kernel_entry

/*
==================================================================================

	Prototype Statement

==================================================================================
*/
/* Open Functions */

/* Local Functions */

/*
==================================================================================

	DEFINE 

==================================================================================
*/



/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< open functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
.text
.code32

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion     :tron_start, _tron_start
	Input       :void
	Output      :void
	Return      :void
	Description :void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
.equ	_tron_start,	( _start - KERNEL_BASE_ADDR ) /* adjust to pa 0x00100000*/
.globl _tron_start
_tron_start:
	jmp		multi_boot_entry

#ifdef MULTIBOOT1
/* align 32 bits boundary	*/
.align 4
#else
/* align 64 bits boundary	*/
.align 8
#endif

/*
==================================================================================

	multiboot header

==================================================================================
*/
multiboot_header1:
	.long	MULTIBOOT_HEADER_MAGIC
	.long	MULTIBOOT_FLAGS
	.long	MULTIBOOT_CHECKSUM
	/* -------------------------------------------------------------------- */
	/* for not ELF								*/
	/* -------------------------------------------------------------------- */
	.long	0	// header_addr
	.long	0	// load_addr
	.long	0	// load_end_addr
	.long	0	// bss_end_addr
	.long	0	// entry_addr
	/* -------------------------------------------------------------------- */
	/* graphics filelds							*/
	/* -------------------------------------------------------------------- */
	.long	0	// entry_addr
	.long	80	// width
	.long	25	// height
	.long	0	// depth

multiboot_header2:
	/* -------------------------------------------------------------------- */
	/* multiboot header 2							*/
	/* -------------------------------------------------------------------- */
	.long	MULTIBOOT_HEADER_MAGIC
	.long	MULTIBOOT_ARCHITECTURE_I386
#if 0
	.long	MULTIBOOT_HEADER_LENGTH
#endif
	.long	MULTIBOOT_HEADER_I386_CHECKSUM
#ifndef __ELF__
	/* -------------------------------------------------------------------- */
	/* for not ELF								*/
	/* -------------------------------------------------------------------- */
address_tag_start:
	.short	MULTIBOOT_HEADER_TAG_ADDRESS
	.short	MULTIBOOT_HEADER_TAG_OPTIONAL
	.long	address_tag_end - address_tag_start
	.long	multiboot_header	// header_addr
	.long	_tron_start		// load_addr
	.long	_edata			// load_end_addr
	.long	_end			// bss_end_addr
address_tag_end:
entry_address_tag_start:
	.short	MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
	.short	MULTIBOOT_HEADER_TAG_OPTIONAL
	.long	entry_address_tag_end - entry_address_tag_start
	.long	multiboot_entry		// entry_addr
entry_address_tag_end:
#endif	// __ELF__
	/* -------------------------------------------------------------------- */
	/* tag : frame buffer							*/
	/* -------------------------------------------------------------------- */
framebuffer_tag_start:
	.short	MULTIBOOT_HEADER_TAG_FRAMEBUFFER
	.short	MULTIBOOT_HEADER_TAG_OPTIONAL
	.long	framebuffer_tag_end - framebuffer_tag_start
	.long	1024
	.long	768
	.long	32
framebuffer_tag_end:
multiboot_header_end:



/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion     :_multi_boot_entry
	Input       :void
	Output      :void
	Return      :void
	Description :void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
multi_boot_entry:
_multi_boot_entry:

	movl	$0x00000003, %ecx
	lea	(_PageTable0 - KERNEL_BASE_ADDR), %edx

_setup_first_ptes:
	movl	%ecx, (%edx)
	addl	$0x4, %edx
	addl	$0x1000, %ecx
	cmp	$(_PageTable0 - KERNEL_BASE_ADDR + 0x4 * KERNEL_PTE_NUM), %edx
	jne	_setup_first_ptes
	
	movl	$0x00000003, %ecx
	lea	(_PageTableC - KERNEL_BASE_ADDR), %edx

_setup_second_ptes:
	movl	%ecx, (%edx)
	addl	$0x4, %edx
	addl	$0x1000, %ecx
	cmp	$(_PageTableC - KERNEL_BASE_ADDR + 0x4 * KERNEL_PTE_NUM), %edx
	jne	_setup_second_ptes

	/* -------------------------------------------------------------------- */
	/* set up paeg( note : use physical address instead of virtual address	*/
	/* untill paging on )							*/
	/* -------------------------------------------------------------------- */
	lea	( _PageDirectory - KERNEL_BASE_ADDR ), %ecx
	mov	%ecx, %cr3
	
	
	/* -------------------------------------------------------------------- */
	/* set pse bit to enable 4MB pages					*/
	/* -------------------------------------------------------------------- */
#	mov	%cr4, %ecx
#	or	$0x00000010, %ecx
#	mov	%ecx, %cr4
	
	/* -------------------------------------------------------------------- */
	/* enable paging							*/
	/* -------------------------------------------------------------------- */
	mov	%cr0, %ecx
	or	$0x80000000, %ecx
	mov	%ecx, %cr0
	
	lea	StartInHighHerHalf, %ecx
	
	jmp	*%ecx
	

StartInHighHerHalf:
	/* -------------------------------------------------------------------- */
	/* initialize the stack pointer						*/
	/* -------------------------------------------------------------------- */
	movl	$(stack + STACK_SIZE), %esp
	
	/* -------------------------------------------------------------------- */
	/* reset eflags								*/
	/* -------------------------------------------------------------------- */
	pushl	$0
	popf
	
	/* -------------------------------------------------------------------- */
	/* push the pointer to the multiboot information			*/
	/* -------------------------------------------------------------------- */
	pushl	%ebx	# 1st argument
	pushl	%eax	# 2nd argument
	
	call	_kernel_entry
	
loop_infinite:
	hlt
	jmp	loop_infinite


#.section .stack
.align 32
stack:
	.rept STACK_SIZE
		.byte 0x00
	.endr

/*
==================================================================================

	page table & directories

==================================================================================
*/
.data
.align 0x1000

_PageTable0:
	.rept ( KERNEL_PTE_NUM )
		.long 0x00000000
	.endr
_PageTableC:
	.rept ( KERNEL_PTE_NUM )
		.long 0x00000000
	.endr

/*
----------------------------------------------------------------------------------
	Page Directories
----------------------------------------------------------------------------------
*/
_PageDirectory:
	/* -------------------------------------------------------------------- */
	/* map first 4MB							*/
	/* bit7:PS=1 page size is 4MB						*/
	/* bit1:RW=1 read/write access						*/
	/* bit0:P =1 page is presence						*/
	/* -------------------------------------------------------------------- */
#	.long	0x00000083	/* first 4MB					*/
#	.rept ( KERNEL_PAGE_NUM - 1 )
#		.long 0x00000000
#	.endr
	/* -------------------------------------------------------------------- */
	/* map 4MB from 0xC0000000						*/
	/* -------------------------------------------------------------------- */
#	.long	0x00000083	/* for 0xC0000000				*/
#	.rept ( 1024 - KERNEL_PAGE_NUM - 1 )
#		.long 0x00000000
#	.endr
	/* -------------------------------------------------------------------- */
	/* map first 4MB							*/
	/* bit7:PS=0 page size is 4KB						*/
	/* bit1:RW=1 read/write access						*/
	/* bit0:P =1 page is presence						*/
	/* -------------------------------------------------------------------- */
	.long	0x00000003 + _PageTable0 - KERNEL_BASE_ADDR	/* first 4MB	*/
	.rept ( KERNEL_PAGE_NUM - 1 )
		.long 0x00000000
	.endr
	/* -------------------------------------------------------------------- */
	/* map 4MB from 0xC0000000						*/
	/* -------------------------------------------------------------------- */
	.long	0x00000003 + _PageTableC - KERNEL_BASE_ADDR	/* for 0xC0000000*/
	.rept ( 1024 - KERNEL_PAGE_NUM - 1 )
		.long 0x00000000
	.endr

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
	Funtion     :void
	Input       :void
	Output      :void
	Return      :void
	Description :void
==================================================================================
*/

