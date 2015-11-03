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
#ifndef __MULTIBOOT__H
#define __MULTIBOOT__H

#ifndef _in_asm_source_

#include <stdint.h>

#include <typedef.h>
#include <sys/sysdepend/std_x86/multiboot_def.h>

/*
=================================================================================

	Multiboot Header

=================================================================================
*/
// MULTIBOOT1
typedef struct
{
	uint32_t	magic;
	uint32_t	flags;
	uint32_t	checksum;
	/* The followings are only valid if MULTIBOOT_AOUT_KLUDGE is set	*/
	uint32_t	header_addr;
	uint32_t	load_addr;
	uint32_t	load_end_addr;
	uint32_t	bss_end_addr;
	uint32_t	entry_addr;
	/* The followings are only valid if MULTIBOOT_VIDEO_MODE is set		*/
	uint32_t	mode_type;
	uint32_t	width;
	uint32_t	height;
	uint32_t	depth;
}MULTIBOOT_HEADER;

// MULTIBOOT2
struct multiboot_header {
	uint32_t	magic;
	uint32_t	architecture;
	uint32_t	header_length;
	uint32_t	checksum;
};

/*
=================================================================================

	Header tag

=================================================================================
*/
// MULTIBOOT2
struct multiboot_header_tag {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
};

// MULTIBOOT2
struct multiboot_header_tag_information_request {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
	uint32_t	requests[0];
};

// MULTIBOOT2
struct multiboot_header_tag_address {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
	uint32_t	header_addr;
	uint32_t	load_addr;
	uint32_t	load_end_addr;
	uint32_t	bss_end_addr;
};

// MULTIBOOT2
struct multboot_header_tag_entry_address {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
	uint32_t	entry_addr;
};

// MULTIBOOT2
struct multiboot_header_tag_console_flags {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
	uint32_t	console_flags;
};

// MULTIBOOT2
struct multiboot_header_tag_framebuffer {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
	uint32_t	width;
	uint32_t	height;
	uint32_t	depth;
};

// MULTIBOOT2
struct multiboot_header_tag_module_align {
	uint16_t	type;
	uint16_t	flags;
	uint32_t	size;
	uint32_t	width;
	uint32_t	height;
	uint32_t	depth;
};

/*
=================================================================================

	Color

=================================================================================
*/
// MULTIBOOT2
struct multiboot_color {
	uint8_t		red;
	uint8_t		green;
	uint8_t		blue;
};

/*
=================================================================================

	Symbol Table for a.out

=================================================================================
*/
// MULTIBOOT1
struct multiboot_aout_symbol_table
{
	UINT32	tabsize;
	UINT32	strsize;
	UINT32	addr;
	UINT32	reserved;
};

/*
=================================================================================

	Section Header Table for ELF

=================================================================================
*/
// MULTIBOOT1
struct multiboot_elf_section_header_table
{
	UINT32	num;
	UINT32	size;
	UINT32	addr;
	UINT32	shndx;
};

/*
=================================================================================

	Multiboot Info

=================================================================================
*/
// MULTIBOOT1
struct multiboot_info
{
	UINT32	flags;			/* multiboot info version number	*/
	/* available memory from BIOS						*/
	UINT32	mem_lower;
	UINT32	mem_upper;
	UINT32	boot_device;		/* "root" partition			*/
	UINT32	cmdline;		/* kernel command line			*/
	/* Boot-Module list							*/
	UINT32	mods_count;
	UINT32	mods_addr;
	
	union
	{
		struct multiboot_aout_symbol_table		aout_sym;
		struct multiboot_elf_section_header_table	elf_sec;
	}u;
	
	/* memory mapping buffer						*/
	UINT32	mmap_length;
	UINT32	mmap_addr;
	/* drive info buffer							*/
	UINT32	drives_length;
	UINT32	drives_addr;
	/* ROM configuration table						*/
	UINT32	config_table;
	/* bootloader name							*/
	UINT32	boot_loader_name;
	/* apm table								*/
	UINT32	apm_table;
	/* Video								*/
	UINT32	vbe_control_info;
	UINT32	vbe_mode_info;
	UINT32	vbe_mode;
	UINT32	vbe_interface_seg;
	UINT32	vbe_interface_off;
	UINT32	vbe_interface_len;
	
};

/*
=================================================================================

	Mmap entry

=================================================================================
*/
// MULTIBOOT1
struct multiboot_entry
{
	UINT32	size;
	UINT64	addr;
	UINT64	len;
#define	DEF_MB_MEMORY_AVAILABLE			1
#define	DEF_MB_MEMORY_RESERVED			2
	UINT32	type;
}__attribute__((packed));

// MULTIBOOT2
struct multiboot_mmap_entry {
	uint64_t	addr;
	uint64_t	len;
#define	MULTIBOOT_MEMORY_AVAILABLE		1
#define	MULTIBOOT_MEMORY_RESERVED		2
#define	MULTIBOOT_MEMORY_ACPI_RECLAIMABLE	3
#define	MULTIBOOT_MEMORY_NVS			4
	uint32_t	type;
	uint32_t	zero;
} __attribute__((packed));

/*
=================================================================================

	Drivers Information

=================================================================================
*/
// MULTIBOOT1
#define	MULTIBOOT_DRIVERS_MODE_CHS	0
#define	MULITBOOT_DRIVERS_MODE_LBA	1

struct multiboot_drives {
	uint32_t	size;
	uint8_t		drive_number;
	uint8_t		drive_mode;
	uint16_t	drive_cylinders;
	uint8_t		drive_heads;
	uint8_t		drive_sectors;
	uint8_t		drive_ports[0];
};

/*
=================================================================================

	Config Table

=================================================================================
*/
struct multiboot_config_table {
	uint16_t	version;
	uint16_t	cseg;
	uint32_t	offset;
	uint16_t	cseg_16;
	uint16_t	dseg;
	uint16_t	flags;
	uint16_t	cseg_len;
	uint16_t	cseg_16_len;
	uint16_t	dseg_len;
};

/*
=================================================================================

	Tag

=================================================================================
*/
// MULTIBOOT2
struct multiboot_tag {
	uint32_t	type;
	uint32_t	size;
};

// MULTIBOOT2
struct multiboot_tag_string {
	uint32_t	type;
	uint32_t	size;
	char		string[0];
};

// MULTIBOOT2
struct multiboot_tag_module {
	uint32_t	type;
	uint32_t	size;
	uint32_t	mod_start;
	uint32_t	mod_end;
	char		cmdline[0];
};

// MULTIBOOT2
struct multiboot_tag_basic_meminfo {
	uint32_t	type;
	uint32_t	size;
	uint32_t	mem_lower;
	uint32_t	mem_upper;
};

// MULTIBOOT2
struct multiboot_tag_bootdev {
	uint32_t	type;
	uint32_t	size;
	uint32_t	biosdev;
	uint32_t	slice;
	uint32_t	part;
};

// MULTIBOOT2
struct multiboot_tag_mmap {
	uint32_t	type;
	uint32_t	size;
	uint32_t	entry_size;
	uint32_t	entry_version;
	struct multiboot_mmap_entry entries[0];
};

/*
=================================================================================

	VBE

=================================================================================
*/
// MULTIBOOT1
struct multiboot_vbe_info {
	uint32_t	vbe_control_info;
	uint32_t	vbe_mode_info;
	uint32_t	vbe_mode;
	uint32_t	vbe_interface_seg;
	uint32_t	vbe_interface_off;
	uint32_t	vbe_interface_len;
};

// MULTIBOOT2
struct multiboot_vbe_info_block {
	uint8_t		external_specification[512];
};

// MULTIBOOT2
struct multiboot_vbe_mode_info_block {
	uint8_t		external_specification[256];
};

// MULTIBOOT2
struct multiboot_tag_vbe {
	uint32_t	type;
	uint32_t	size;
	uint16_t	vbe_mode;
	uint16_t	vbe_interface_seg;
	uint16_t	vbe_interface_off;
	uint16_t	vbe_interface_len;

	struct multiboot_vbe_info_block vbe_control_info;
	struct multiboot_vbe_mode_info_block vbe_mode_info;
};

// MULTIBOOT2
struct multiboot_tag_framebuffer_common {
	uint32_t	type;
	uint32_t	size;

	uint64_t	framebuffer_addr;
	uint32_t	framebuffer_pitch;
	uint32_t	framebuffer_width;
	uint32_t	framebuffer_height;
	uint8_t		framebuffer_bpp;
#define	MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED	0
#define	MULTIBOOT_FRAMEBUFFER_TYPE_RGB		1
#define	MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXGT	2
	uint8_t		framebuffer_type;
	uint16_t	reserved;
};

// MULTIBOOT2
struct multiboot_tag_framebuffer {
	struct multiboot_tag_framebuffer_common	commn;

	union {
		struct {
			uint16_t	framebuffer_palette_num_colors;
			struct multiboot_color framebuffer_palette[0];
		};

		struct {
			uint8_t		framebuffer_red_field_position;
			uint8_t		framebuffer_red_mask_size;
			uint8_t		framebuffer_green_field_position;
			uint8_t		framebuffer_green_mask_size;
			uint8_t		framebuffer_blue_field_position;
			uint8_t		framebuffer_blue_mask_size;
		};
	};
};


/*
=================================================================================

	ELF

=================================================================================
*/
// MULTIBOOT1
struct multiboot_elf_sections {
	uint32_t	num;
	uint32_t	size;
	uint32_t	shndx;
};

// MULTIBOOT2
struct multiboot_tag_elf_sections {
	uint32_t	type;
	uint32_t	size;
	uint32_t	num;
	uint32_t	entsize;
	uint32_t	shndx;
	char		sections[0];
};

/*
=================================================================================

	APM

=================================================================================
*/
// MULTIBOOT2
struct multiboot_tag_apm {
	uint32_t	type;
	uint32_t	size;
	uint16_t	version;
	uint16_t	cseg;
	uint32_t	offset;
	uint16_t	cseg_16;
	uint16_t	dseg;
	uint16_t	flags;
	uint16_t	cseg_len;
	uint16_t	cseg_16_len;
	uint16_t	dseg_len;
};

/*
=================================================================================

	Module list

=================================================================================
*/
#define	MULTIBOOT_INITRAMFS	0

// MULTIBOOT1
struct multiboot_mod_list
{
	/* the memory used goes from bytes 'mod_start' to 'mode_end_1' inclusive*/
	UINT32	mod_start;
	UINT32	mod_end;
	/* module command line							*/
	UINT32	cmdline;
	/* padding to take it to 16bytes (must be zero)				*/
	UINT32	pad;
};

// MULTIBOOT2
struct multiboot_mod_list2
{
	uint32_t	size;
	uint32_t	paddress;
	char		string[0];
};


/*
=================================================================================

	Boot Information

=================================================================================
*/
// MULTIBOOT1 and MULTIBOOT2
struct boot_info
{
	uint32_t	version;	/* multiboot info version number(1,2)	*/
	/* available memory from BIOS						*/
	uint32_t	mem_lower;	/* (1,2)				*/
	uint32_t	mem_upper;	/* (1,2)				*/
	uint32_t	boot_device;	/* "root" partition(1,2:biosdev)	*/
	uint32_t	partition2;	/* (2)					*/
	uint32_t	sub_partition2;	/* (2)					*/
	char		*cmdline;	/* kernel command line(1,2)		*/
	/* Boot-Module list(1)							*/
	uint32_t	mods_count;	/* (1,2)				*/
	struct multiboot_mod_list *mod_list;	/* (1)				*/
	/* Modules(2)								*/
	struct multiboot_mod_list2 *mod_list2;	/* (2)				*/
	
	union
	{
		struct multiboot_aout_symbol_table		*aout_sym;
		struct multiboot_elf_section_header_table	*elf_sec;/* (1)	*/
		struct multiboot_tag_elf_sections		*elf_sec2;/*(2)	*/
	}u;
	
	/* memory mapping buffer						*/
	uint32_t	num_mmap_entries;	/* (1,2)			*/
	struct multiboot_entry *mmap;		/* (1)				*/
	struct multiboot_mmap_entry *mmap2;	/* (2)				*/
	/* boot loader name(2)							*/
	char		*boot_loader_name;	/* (1,2)			*/
	/* drive info buffer(1)							*/
	uint32_t	drives_length;
	struct multiboot_drives *drives;
	/* ROM configuration table(1)						*/
	struct multiboot_config_table	*config_table;

	/* APM table(1,2)							*/
	void			 *apm_table;	/* (1)				*/
	struct multiboot_tag_apm *apm_table2;	/* (2)				*/
	/* VBE info(1,2)							*/
	struct multiboot_vbe_info	*vbe_info;
	struct multiboot_tag_vbe	*vbe_info2;
	/* Framebuffer info(2)							*/
	struct multiboot_tag_framebuffer *framebuffer_info2;
	/* available memory(1,2)						*/
	unsigned long lowmem_top;
	unsigned long lowmem_limit;
	unsigned long lowmem_base;
} ;


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< open functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getBootInfo
 Input		:void
 Output		:void
 Return		:struct boot_info*
 		 < boot information >
 Description	:get boot information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT __inline__ struct boot_info* getBootInfo( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:isInitramfs
 Input		:void
 Output		:void
 Return		:int
 		 < 0:not exist >
 Description	:is there initramfs?
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL __inline__ int isInitramfs(void)
{
	return(getBootInfo()->mods_count);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getInitramfsAddress
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < start address of initramfs >
 Description	:get start address of initramfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL __inline__ uint32_t getInitramfsAddress(void)
{
	return(getBootInfo()->mod_list[MULTIBOOT_INITRAMFS].mod_start);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getInitramfsSize
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < size of initramfs >
 Description	:get size of initramfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL __inline__ uint32_t getInitramfsSize(void)
{
	return(getBootInfo()->mod_list[MULTIBOOT_INITRAMFS].mod_end - 
		getBootInfo()->mod_list[MULTIBOOT_INITRAMFS].mod_start);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setInitramfsAddress
 Input		:uint32_t start
		 < start address of initramfs >
		 uint32_t size
		 < size of initramfs >
 Output		:void
 Return		:void
 Description	:set start and end address of initramfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL __inline__ void setInitramfsAddress(uint32_t start, uint32_t size)
{
	getBootInfo()->mod_list[MULTIBOOT_INITRAMFS].mod_end = 
		getBootInfo()->mod_list[MULTIBOOT_INITRAMFS].mod_start + size;
	getBootInfo()->mod_list[MULTIBOOT_INITRAMFS].mod_start = start;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:allocLowMemory
 Input		:uint32_t size
		 < size to allocate >
 Output		:void
 Return		:void*
		 < allocated memory address >
 Description	:allocate a memory area from lowmem
		 this method only use for boot sequence after analyzing memory map
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void* allocLowMemory(uint32_t size);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copyInitramfs
 Input		:void
 Output		:void
 Return		:int
		 < status >
 Description	:copy initramfs on the unmanaged page to the safe memory area
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int copyInitramfs(void);

#endif	/* __in_asm_source_		*/
#endif	/* __MULTIBOOT__H		*/
