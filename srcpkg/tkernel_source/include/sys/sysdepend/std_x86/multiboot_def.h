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
#ifndef __MULTIBOOT_DEF__H
#define __MULTIBOOT_DEF__H

#define MULTIBOOT1

/*
==================================================================================

	DEFINE for C and Assember

==================================================================================
*/

#define MULTIBOOT_HEADER_MAGIC		0x1BADB002
#define	MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002/* The magic should be in EAX	*/
#define	MULTIBOOT2_HEADER_MAGIC		0xE85250D6
#define	MULTIBOOT2_BOOTLOADER_MAGIC	0x36D76289/* The magic should be in EAX	*/


/* How many bytes from the start of the file searched for the header		*/
#define	MULTIBOOT_SEARCH		8192
#define	MULITBOOT_HEADER_ALIGN		8

/* The bits in the required part of flags field are supported			*/
#define	MULTIBOOT_UNSUPPORTED		0x0000FFFC

/* Alignment of multiboot modules						*/
#define	MULTIBOOT_MOD_ALIGN		0x00001000

/* Alignment of the multiboot info structure					*/
//#define	MULTIBOOT_INFO_ALIGN		0x00000004
#define	MULTIBOOT_INFO_ALIGN		0x00000008

#define MULITBOOT_KB_SIZE		1024

/* kernel base address								*/
#define	KERNEL_BASE_ADDR		0xC0000000
#define	KERNEL_PAGE_NUM			( KERNEL_BASE_ADDR >> 22 )
#define	KERNEL_PTE_NUM			1024
#define	STACK_SIZE			0x00004000


/* Flags set in the 'flags'member of the multiboot header			*/
#define	MULTIBOOT_TAG_ALIGN		8
#define	MULTIBOOT_TAG_TYPE_END		0
#define	MULTIBOOT_TAG_CMDLINE		1
#define	MULTIBOOT_TAG_BOOT_LOADER_NAME	2
#define	MULTIBOOT_TAG_MODULE		3
#define	MULTIBOOT_TAG_BASIC_MEMINFO	4
#define	MULTIBOOT_TAG_BOOTDEV		5
#define	MULTIBOOT_TAG_TYPE_MMAP		6
#define	MULTIBOOT_TAG_VBE		7
#define	MULTIBOOT_TAG_FRAMEBUFFER	8
#define	MULTIBOOT_TAG_ELF_SECTIONS	9
#define	MULTIBOOT_TAG_APM		10

#define	MULTIBOOT_HEADER_TAG_END			0
#define	MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST	1
#define	MULTIBOOT_HEADER_TAG_ADDRESS			2
#define	MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS		3
#define	MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS		4
#define	MULTIBOOT_HEADER_TAG_FRAMEBUFFER		5
#define	MULTIBOOT_HEADER_TAG_MODULE_ALIGN		6

#define	MULTIBOOT_ARCHITECTURE_I386	0
#define	MULTIBOOT_ARCHITECTURE_MIPS32	4

#define	MULTIBOOT_HEADER_TAG_OPTIONAL	1


#define	MULTIBOOT_HEADER_LENGTH		(multiboot_header_end - multiboot_header)
#if 0
#define	MULTIBOOT_HEADER_I386_CHECKSUM						\
	(-(MULTIBOOT2_HEADER_MAGIC						\
		+ MULTIBOOT_ARCHITECTURE_I386					\
		+ MULTIBOOT_HEADER_LENGTH ) )
#endif
#define	MULTIBOOT_HEADER_I386_CHECKSUM						\
	(-(MULTIBOOT2_HEADER_MAGIC						\
		+ MULTIBOOT_ARCHITECTURE_I386					\
		 )  )

/*
----------------------------------------------------------------------------------

	Flags of Multiboot Header

----------------------------------------------------------------------------------
*/
#define	MULTIBOOT_PAGE_ALIGN		0x00000001	/* page (4KB) boundaries*/
#define	MULTIBOOT_MEMORY_INFO		0x00000002	/* memory information	*/
#define	MULTIBOOT_VIDEO_MODE		0x00000004	/* video mode		*/
#define	MULTIBOOT_AOUT_KLUDGE		0x00010000	/* use address fields	*/

/*
----------------------------------------------------------------------------------

	Flags of Multiboot Info structure

----------------------------------------------------------------------------------
*/
/* lower/upper memory info bit							*/
#define	MULTIBOOT_INFO_MEMORY		0x00000001
/* is there a boot device set?							*/
#define	MULTIBOOT_INFO_BOOTDEV		0x00000002
/* command-line defined?							*/
#define	MULTIBOOT_INFO_CMDLINE		0x00000004
/* modules to do something with?						*/
#define	MULTIBOOT_INFO_MODS		0x00000008
/* a symbol table loaded?							*/
#define	MULTIBOOT_INFO_AOUT_SYMS	0x00000010
/* ELF section header table?							*/
#define	MULTIBOOT_INFO_ELF_SHDR		0x00000020
/* full memory map?								*/
#define	MULTIBOOT_INFO_MEM_MAP		0x00000040
/* drive inofo?									*/
#define	MULTIBOOT_INFO_DRIVE_INFO	0x00000080
/* is there a config table?							*/
#define	MULTIBOOT_INFO_CONFIG_TABLE	0x00000100
/* is there a boot loader name?							*/
#define	MULTIBOOT_INFO_BOOT_LOADER_NAME	0x00000200
/* an APM table?								*/
#define	MULTIBOOT_INFO_APM_TABLE	0x00000400
/* video information?								*/
#define	MULTIBOOT_INFO_VIDEO_INFO	0x00000800


/*
=================================================================================

	Definition of multiboot header

=================================================================================
*/
#define	MULTIBOOT_FLAGS	MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO

	/* -------------------------------------------------------------------- */
	/* for not ELF								*/
	/* -------------------------------------------------------------------- */
#define	MULTIBOOT_CHECKSUM	( - ( MULTIBOOT_HEADER_MAGIC + MULTIBOOT_FLAGS ) )
#define	MBH_MODE_TYPE		0x00000001
#define	MBH_WIDTH		0x00000050
#define	MBH_HEIGHT		0x00000028
#define	MBH_DEPTH		0x00000000

#endif	/* __MULTIBOOT_DEF__H */
