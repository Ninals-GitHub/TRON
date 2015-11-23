/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *    Modified by Nina Petipa at 2015/10/18
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	elf.h
 *
 *       T2EX: program load functions
 *       ELF (Executable and Linking Format) definitions
 */

#ifndef _T2EX_LOAD_ELF_
#define _T2EX_LOAD_ELF_

#include <basic.h>
#include <stdint.h>

typedef uint32_t	Elf32_Word;
typedef uintptr_t	Elf32_Addr;

/* ELF header */
#define	EI_NIDENT	16
typedef struct {
	UB	e_ident[EI_NIDENT];	/* ELF identifier */
	UH	e_type;			/* object file type */
	UH	e_machine;		/* machine architecture */
	UW	e_version;		/* file format & version */
	VP	e_entry;		/* entry point address */
	UW	e_phoff;		/* program header offset */
	UW	e_shoff;		/* section header offset */
	UW	e_flags;		/* processor-specific flags (EF_xxx) */
	UH	e_ehsize;		/* ELF header size */
	UH	e_phentsize;		/* program header entry size */
	UH	e_phnum;		/* the number of program header entries */
	UH	e_shentsize;		/* section header entry size */
	UH	e_shnum;		/* the number of section header entries */
	UH	e_shstrndx;		/* section header string index */
} Elf32_Ehdr;

/* e_ident[] indices */
#define	EI_MAG0		0	/* magic number */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4	/* file class */
#define	EI_DATA		5	/* data encoding */
#define	EI_VERSION	6	/* ELF version */
#define	EI_OSABI	7	/* Operating system/ABI identification */
#define	EI_ABIVERSION	8	/* ABI version */
#define	EI_PAD		9	/* start of padding bytes */

/* EI_MAG */
#define	ELFMAG0		0x7f
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

/* EI_CLASS */
#define	ELFCLASSNONE	0	/* invalid */
#define	ELFCLASS32	1	/* 32-bit object */
#define	ELFCLASS64	2	/* 64-bit object */
#define	ELFCLASSNUM	3

/* EI_DATA */
#define	ELFDATANONE	0	/* invalid */
#define	ELFDATA2LSB	1	/* two's complement, little endian */
#define	ELFDATA2MSB	2	/* two's complement, big endian */
#define	ELFDATANUM	3

/* e_type */
#define	ET_NONE		0	/* unknown */
#define	ET_REL		1	/* relocatable format */
#define	ET_EXEC		2	/* executable format */
#define	ET_DYN		3	/* shared object format */
#define	ET_CORE		4	/* core format */
#define	ET_NUM		5
#define	ET_LOOS		0xFE00	/* OS-specific */
#define	ET_LOSNUM	0xFEFF
#define	ET_SUNWPSEUDO	0xFEFF
#define	ET_HISUNW	0xFEFF
#define	ET_HIOS		0xFEFF
#define	ET_LOPROC	0xFF00	/* CPU-specific */
#define	ET_HIPROC	0xFFFF

/* e_machine */
#define	EM_NONE		0	/* unknown */
#define	EM_M32		1	/* AT&T WE 32100 */
#define	EM_SPARC	2	/* Sun SPARC */
#define	EM_386		3	/* Intel 80386 */
#define	EM_68K		4	/* Motorola 68000 */
#define	EM_88K		5	/* Motorola 88000 */
#define	EM_486		6	/* Intel 80486 */
#define	EM_860		7	/* Intel i860 */
#define EM_MIPS		8	/* MIPS R3000 Big-Endian */
#define	EM_S370		9	/* IBM System/370 Processor */
#define	EM_MIPS_RS3_LE	10	/* MIPS RS3000 Little-Endian */
#define	EM_RS6000	11	/* RS6000 */
#define	EM_UNKNOWN12	12
#define	EM_UNKNOWN13	13
#define	EM_UNKNOWN14	14
#define	EM_PA_RISC	15	/* PA-RISC */
#define	EM_PARISC	EM_PA_RISC /* Alias: GNU compatibility */
#define	EM_nCUBE	16	/* nCUBE */
#define	EM_VPP500	17	/* Fujitsu VPP500 */
#define	EM_SPARC32PLUS	18	/* Sun SPARC 32+ */
#define	EM_PPC		20	/* PowerPC */
#define	EM_PPC64	21	/* 64-bit PowerPC */
#define	EM_S390		22	/* IBM System/390 Processor */
#define	EM_UNKNOWN22	EM_S390	/* Alias: Older published name */
#define	EM_UNKNOWN23	23
#define	EM_UNKNOWN24	24
#define	EM_UNKNOWN25	25
#define	EM_UNKNOWN26	26
#define	EM_UNKNOWN27	27
#define	EM_UNKNOWN28	28
#define	EM_UNKNOWN29	29
#define	EM_UNKNOWN30	30
#define	EM_UNKNOWN31	31
#define	EM_UNKNOWN32	32
#define	EM_UNKNOWN33	33
#define	EM_UNKNOWN34	34
#define	EM_UNKNOWN35	35
#define	EM_V810		0x24	/* NEC V810 */
#define	EM_FR20		37	/* Fujitsu FR20 */
#define	EM_RH32		38	/* TRW RH-32 */
#define	EM_RCE		39	/* Motorola RCE */
#define EM_ARM		40	/* ARM */
#define	EM_ALPHA	41	/* Digital Alpha */
#define	EM_SH		42	/* Hitachi SH */
#define	EM_SPARCV9	43	/* Sun SPARC V9 (64bit) */
#define	EM_TRICORE	44	/* Siemens Tricore embedded processor */
#define	EM_ARC		45	/* Argonaut RISC Core
				   Argonaut Technologies Inc. */
#define	EM_H8_300	46	/* Hitachi H8/300 */
#define	EM_H8_300H	47	/* Hitachi H8/300H */
#define	EM_H8S		48	/* Hitachi H8S */
#define	EM_H8_500	49	/* Hitachi H8/500 */
#define	EM_IA_64	50	/* Intel IA64 */
#define	EM_MIPS_X	51	/* Stanford MIPS-X */
#define	EM_COLDFIRE	52	/* Motorola ColdFire */
#define	EM_68HC12	53	/* Motorola M68HC12 */
#define	EM_MMA		54	/* Fujitsu MMA Multimedia Accelerator */
#define	EM_PCP		55	/* Siemens PCP */
#define	EM_NCPU		56	/* Sony nCPU embedded RISC processor */
#define	EM_NDR1		57	/* Denso NDR1 microprocessor */
#define	EM_STARCORE	58	/* Motorola Start*Core processor */
#define	EM_ME16		59	/* Toyota ME16 processor */
#define	EM_ST100	60	/* STMicroelectronics ST100 processor */
#define	EM_TINYJ	61	/* Advanced Logic Corp. TinyJ embedded processor family */
#define	EM_AMD64	62	/* AMDs x86-64 architecture */
#define	EM_X86_64	EM_AMD64 /* for compatibility */
#define	EM_PDSP		63	/* Sony DSP Processor */
#define	EM_UNKNOWN64	64
#define	EM_UNKNOWN65	65
#define	EM_FX66		66	/* Siemens FX66 microcontroller */
#define	EM_ST9PLUS	67	/* STMicroelectronics ST9+8/16 bit microcontroller */
#define	EM_ST7		68	/* STMicroelectronics ST7 8 bit microcontroller */
#define	EM_68HC16	69	/* Motorola MC68HC16 microcontroller */
#define	EM_68HC11	70	/* Motorola MC68HC11 microcontroller */
#define	EM_68HC08	71	/* Motorola MC68HC08 microcontroller */
#define	EM_68HC05	72	/* Motorola MC68HC05 microcontroller */
#define	EM_SVX		73	/* Silicon Graphics SVx */
#define	EM_ST19		74	/* STMicroelectronics ST19 8 bit microcontroller */
#define	EM_VAX		75	/* Digital VAX */
#define	EM_CRIS		76	/* Axis Communications 32 bit embedded processor */
#define	EM_JAVELIN	77	/* Infineon Technologies 32 bit embedded processor */
#define	EM_FIREPATH	78	/* Element 14 64 bit DSP processor */
#define	EM_ZSP		79	/* LSI Logic 16 bit DSP processor */
#define	EM_MMIX		80	/* Donald Knuth's educational 64 bit processor */
#define	EM_HUANY	81	/* harvard University machine-independent object files*/
#define	EM_PRISM	82	/* SiTera Prism */
#define	EM_AVR		83	/* Atmel AVR 8 bit microcontroller */
#define	EM_FR30		84	/* Fujitsu FR30 */
#define	EM_D10V		85	/* Mitsubishi D10V */
#define	EM_D30V		86	/* Mitsubishi D30V */
#define	EM_V850		87	/* NEC V850 */
#define	EM_M32R		88	/* Mitsubishi M32R */
#define	EM_MN10300	89	/* Matsushita MN10300 */
#define	EM_MN10200	90	/* Matsushita MN10200 */
#define	EM_PJ		91	/* picoJava */
#define	EM_OPENRISC	92	/* OpenRISC 32 bit embedded processor */
#define	EM_ARC_A5	93	/* ARC Cores Tangent-A5 */
#define	EM_XTENSA	94	/* Tensilica Xtensa architecture */
#define	EM_NUM		95
#define	EM_NIOS2	0x71	/* Nios2 */

/* e_version, EI_VERSION */
#define	EV_NONE		0	/* unknown */
#define	EV_CURRENT	1	/* current version */

#define	ELFOSABI_NONE		0	/* nno extensions or unspecified */
#define	ELFOSABI_SYSV		ELFOSABI_NONE
#define	ELFOSABI_HPUX		1	/* Hewlett-Packard HP-UX */
#define	ELFOSABI_NETBSD		2	/* NetBSD */
#define	ELFOSABI_LINUX		3
#define	ELFOSABI_UNKNOWN4	4
#define	ELFOSABI_UNKNOWN5	5
#define	ELFOSABI_SOLARIS	6	/* Sun Solaris */
#define	ELFOSABI_AIX		7
#define	ELFOSABI_IRIX		8
#define	ELFOSABI_FREEBSD	9
#define	ELFOSABI_TRU64		10	/* Compaq TRU64 UNIX */
#define	ELFOSABI_MODESTO	11	/* Novell Modestro */
#define	ELFOSABI_OPENBSD	12
#define	ELFOSABI_OPENVMS	13
#define	ELFOSABI_NSK		14	/* Hewlett-Packard Non-Stop Kernel */
#define	ELFOSABI_AROS		15	/* Amiga Reserach OS */
#define	ELFOSABI_ARM		97	/* ARM */
#define	ELFOSABI_STANDALONE	255	/* standalone (embedded) application */

/* Section header */
typedef struct {
	UW	sh_name;	/* section name index */
	UW	sh_type;	/* section type (SHT_xxx) */
	UW	sh_flags;	/* section flags (SHF_xxx) */
	VP	sh_addr;	/* section address */
	UW	sh_offset;	/* section offset */
	UW	sh_size;	/* section size */
	UW	sh_link;	/* section header table index link */
	UW	sh_info;	/* extra information */
	UW	sh_addralign;	/* alignment */
	UW	sh_entsize;	/* entry size */
} Elf32_Shdr;

/* sh_name: special section indices */
#define	SHN_UNDEF	0	/* undefined */
#define	SHN_LORESERVE	0xff00	/* lower bound of the reserved indices range */
#define	SHN_ABS		0xfff1	/* absolute values */
#define	SHN_COMMON	0xfff2	/* common symbols */
#define	SHN_HIRESERVE	0xffff	/* upper bound of the reserved indices range */
#define	SHN_LOPROC	0xff00	/* CPU-specific */
#define	SHN_HIPROC	0xff1f
#define	SHN_LOOS	0xFF20	/* OS specific range */
#define	SHN_LOSUNW	0xFF3F
#define	SHN_SUNW_IGNORE	0xFF3F
#define	SHN_HISUNW	0xFF3F
#define	SHN_HIOS	0xFF3F
#define	SHN_MACHO_64	0xFFFD
#define	SHN_MACHO	0xFFFE
#define	SHN_XINDEX	0xFFFF

/* sh_type */
#define	SHT_NULL		0	/* invalid section */
#define	SHT_PROGBITS		1	/* program */
#define	SHT_SYMTAB		2	/* symbol table */
#define	SHT_STRTAB		3	/* string table */
#define	SHT_RELA		4	/* relocation information, w/ explicit addends */
#define	SHT_HASH		5	/* symbol hash table */
#define	SHT_DYNAMIC		6	/* dynamic-link information */
#define	SHT_NOTE		7	/* note */
#define	SHT_NOBITS		8	/* empty section */
#define	SHT_REL			9	/* relocation information, w/o explicit addends */
#define	SHT_SHLIB		10	/* (reserved) */
#define	SHT_DYNSYM		11	/* symbol table for dynamic links */
#define	SHT_UNKNOWN12		12
#define	SHT_UNKNOWN13		13
#define	SHT_INIT_ARRAY		14
#define	SHT_FINI_ARRAY		15
#define	SHT_PREINIT_ARRAY	16
#define	SHT_GROUP		17
#define	SHT_SYMTAB_SHNDX	18
#define	SHT_NUM			19
#define	SHT_LOOS		0x60000000	/* OS specific range */
#define	SHT_LOSUNW		0x6ffffff1
#define	SHT_SUNW_symsort	0x6ffffff1
#define	SHT_SUNW_tlssort	0x6ffffff2
#define	SHT_SUNW_LDYNSYM	0x6ffffff3
#define	SHT_SUNW_dof		0x6ffffff4
#define	SHT_SUNW_cap		0x6ffffff5
#define	SHT_SUNW_SIGNATURE	0x6ffffff6
#define	SHT_SUNW_ANNOTATE	0x6ffffff7
#define	SHT_SUNW_DEBUGSTR	0x6ffffff8
#define	SHT_SUNW_DEBUG		0x6ffffff9
#define	SHT_SUNW_move		0x6ffffffa
#define	SHT_SUNW_COMDAT		0x6ffffffb
#define	SHT_SUNW_syminfo	0x6ffffffc
#define	SHT_SUNW_verdef		0x6ffffffd
#define	SHT_SUNW_verneed	0x6ffffffe
#define	SHT_SUNW_versym		0x6fffffff
#define	SHT_HISUNW		0x6fffffff
#define	SHT_HIOS		0x6fffffff

#define	SHT_GNU_verdef		0x6ffffffd
#define	SHT_GNU_verneed		0x6ffffffe
#define	SHT_GNU_versym		0x6fffffff

#define	SHT_LOUSER		0x80000000	/* application-specific */
#define	SHT_HIUSER		0xffffffff
#define	SHT_LOPROC		0x70000000	/* CPU-specific */
#define	SHT_HIPROC		0x7fffffff
#define SHT_MIPS_REGINFO	0x70000006	/* MIPS-specific */

/* sh_flags */
#define	SHF_WRITE		0x1	/* writable during execution */
#define	SHF_ALLOC		0x2	/* occupied during execution */
#define	SHF_EXECINSTR		0x4	/* contains executable instructions */
#define	SHF_MERGE		0x10
#define	SHF_STRINGS		0x20
#define	SHF_INFO_LINK		0x40
#define	SHF_LINK_ORDER		0x80
#define	SHF_OS_NONCONFORMING	0x100
#define	SHF_GROUP		0x200
#define	SHF_TLS			0x400

#define	SHF_MASKPROC		0xf0000000	/* CPU-specific */
#define	SHF_MASKOS		0x0FF00000	/* OS specific values */


/* Symbol table */
typedef struct {
	UW	st_name;	/* symbol name index */
	VP	st_value;	/* value */
	UW	st_size;	/* size */
	UB	st_info;	/* symbol type & binding attributes */
	UB	st_other;	/* reserved (0) */
	UH	st_shndx;	/* section index */
} Elf32_Sym;

#define	STN_UNDEF	0	/* undefined */

/* st_info */
#define	ELF32_ST_BIND(info)		((UB)(info) >> 4)
#define	ELF32_ST_TYPE(info)		((UB)(info) & 0xf)
#define	ELF32_ST_INFO(bind, type)	(UB)(((bind) << 4) + ((type) & 0xf))

/* st_info : bind */
#define	STB_LOCAL	0	/* local symbol */
#define	STB_GLOBAL	1	/* global symbol */
#define	STB_WEAK	2	/* weak global symbol */
#define	STB_NUM		3
#define	STB_LOPROC	13	/* CPU-specific */
#define	STB_HIPROC	15

/* st_info : type */
#define	STT_NOTYPE	0	/* undefined */
#define	STT_OBJECT	1	/* data */
#define	STT_FUNC	2	/* function */
#define	STT_SECTION	3	/* section */
#define	STT_FILE	4	/* file name */
#define	STT_COMMON	5
#define	STT_TLS		6
#define	STT_NUM		7
#define	STT_LOPROC	13	/* CPU-specific */
#define	STT_HIPROC	15

#define	ELF32_ST_VISIBILITY(other)	((other)&0x7)
#define	ELF64_ST_VISIBILITY(other)	((other)&0x7)

#define	STV_DEFAULT	0
#define	STV_INTERNAL	1
#define	STV_HIDDEN	2
#define	STV_PROTECTED	3
#define	STV_EXPORTED	4
#define	STV_SINGLETON	5
#define	STV_ELIMINATE	6

/* st_other */
#define STO_MIPS16	0xf0	/* MIPS16 */


/* Relocation */
typedef struct {
	VP	r_offset;	/* relocation target offset */
	UW	r_info;		/* symbol number & relocation type */
} Elf32_Rel;

typedef struct {
	VP	r_offset;	/* relocation target offset */
	UW	r_info;		/* symbol number & relocation type */
	W	r_addend;	/* addend */
} Elf32_Rela;

/* r_info */
#define	ELF32_R_SYM(info)	((UW)(info)>>8)
#define	ELF32_R_TYPE(info)	((UB)(info))
#define	ELF32_R_INFO(sym, type)	(UW)(((sym) << 8) + (UB)(type))

/* i386 Relocation Type */
#define	R_386_NONE	0
#define	R_386_32	1
#define	R_386_PC32	2
#define	R_386_GOT32	3
#define	R_386_PLT32	4
#define	R_386_COPY	5
#define	R_386_GLOB_DAT	6
#define	R_386_JMP_SLOT	7
#define	R_386_RELATIVE	8
#define	R_386_GOTOFF	9
#define	R_386_GOTPC	10
#define	R_386_32PLT	11
#define	R_386_TLS_TPOFF		14	/* offset in static TLS block		*/
#define	R_386_TLS_IE		15	/* address of GOT entry for static TLS	*/
					/* block offset				*/
#define	R_386_TLS_GOTIE		16	/* GOT entry for static TLS block	*/
#define	R_386_TLS_LE		17	/* offset relative to static TLS block	*/
#define	R_386_TLS_GD		18	/* direct 32 bit for GNU version of	*/
					/* general dynamic thread local data	*/
#define	R_386_TLS_LDM		19	/* direct 32 bit for GNU version of local*/
					/* dynamic thread local data in LE code	*/
#define	R_386_16		20
#define	R_386_PC16		21
#define	R_386_8			22
#define	R_386_PC8		23
#define	R_386_TLS_GD_32		24	/* direct 32 bit for general dynamic	*/
					/* thread local data			*/
#define	R_386_TLS_GD_PUSH	25	/* tag for pushl in GD TLS code		*/
#define	R_386_TLS_GD_CALL	26	/* relocation for call to __tls_get_addr*/
#define	R_386_TLS_GD_POP	27	/* tag for popl in GD TLS code		*/
#define	R_386_TLS_LDM_32	28	/* direct 32 bit for local dynamic	*/
					/* thread local data in LE code		*/
#define	R_386_TLS_LDM_PUSH	29	/* tag for pushl in LDM TLS code	*/
#define	R_386_TLS_LDM_CALL	30	/* relocation for call to __tls_get_addr*/
					/* in LDM code				*/
#define	R_386_TLS_LDM_POP	31	/* tag for popl in LDM TLS code		*/
#define	R_386_TLS_LDO_32	32	/* offset relative to TLS block		*/
#define	R_386_TLS_IE_32		33	/* GOT entry for negated static TLS	*/
					/* block offset				*/
#define	R_386_TLS_LE_32		34	/* negated offset relative to static	*/
					/* TLS block				*/
#define	R_386_TLS_DTPMOD32	35	/* ID of module containig symbol	*/
#define	R_386_TLS_DTPOFF32	36	/* offset in TLS block			*/
#define	R_386_TLS_GOTDESC	39	/* GOT offset for TLS descriptor	*/
#define	R_386_TLS_DESC_CALL	40	/* marker of call through TLS descriptor*/
					/* for relaxation			*/
#define	R_386_TLS_DESC		41	/* TLS descriptor containing pointer	*/
					/* to code and to argument, returning	*/
					/* the TLS offset for the symbol	*/
#define	R_386_IRELATIVE		42	/* adjust indirectly by program base	*/
#define	R_386_NUM		43


/* SH Relocation Type */
#define R_SH_NONE	0	/* No relocation */
#define R_SH_DIR32	1	/* 32 bit absolute relocation */
#define R_SH_REL32	2	/* 32 bit PC relative relocation */
#define R_SH_DIR8WPN	3	/* 8 bit PC relative branch divided by 2 */
#define R_SH_IND12W	4	/* 12 bit PC relative branch divided by 2 */
#define R_SH_DIR8WPL	5	/* 8 bit unsigned PC relative divided by 4 */
#define R_SH_DIR8WPZ	6	/* 8 bit unsigned PC relative divided by 2 */
#define R_SH_DIR8BP	7	/* 8 bit GBR relative */
#define R_SH_DIR8W	8	/* 8 bit GBR relative divided by 2 */
#define R_SH_DIR8L	9	/* 8 bit GBR relative divided by 4 */
#define R_SH_GOT32	160
#define R_SH_PLT32	161
#define R_SH_COPY	162
#define R_SH_GLOB_DAT	163
#define R_SH_JMP_SLOT	164
#define R_SH_RELATIVE	165

/* ARM Relocation Type */
#define R_ARM_NONE	0
#define R_ARM_PC24	1
#define R_ARM_ABS32	2
#define R_ARM_REL32	3
#define R_ARM_THM_PC22	10
#define R_ARM_COPY	20	/* Copy symbol at runtime.  */
#define R_ARM_GLOB_DAT	21	/* Create GOT entry.  */
#define R_ARM_JUMP_SLOT	22	/* Create PLT entry.  */
#define R_ARM_RELATIVE	23	/* Adjust by program base.  */
#define R_ARM_GOTOFF	24	/* 32 bit offset to GOT.  */
#define R_ARM_GOTPC	25	/* 32 bit PC relative offset to GOT.  */
#define R_ARM_GOT32	26	/* 32 bit GOT entry.  */
#define R_ARM_PLT32	27	/* 32 bit PLT address.  */

/* MIPS Relocation Type */
#define R_MIPS_NONE	0
#define R_MIPS_16	1
#define R_MIPS_32	2
#define R_MIPS_REL32	3
#define R_MIPS_26	4
#define R_MIPS_HI16	5
#define R_MIPS_LO16	6
#define R_MIPS_GPREL16	7
#define R_MIPS_LITERAL	8
#define R_MIPS16_26	100
#define R_MIPS16_GPREL	101

/* PowerPC Relocations Type */
#define	R_PPC_NONE		0
#define	R_PPC_ADDR32		1
#define	R_PPC_ADDR24		2
#define	R_PPC_ADDR16		3
#define	R_PPC_ADDR16_LO		4
#define	R_PPC_ADDR16_HI		5
#define	R_PPC_ADDR16_HA		6
#define	R_PPC_ADDR14		7
#define	R_PPC_ADDR14_BRTAKEN	8
#define	R_PPC_ADDR14_BRNTAKEN	9
#define	R_PPC_REL24		10
#define	R_PPC_REL14		11
#define	R_PPC_REL14_BRTAKEN	12
#define	R_PPC_REL14_BRNTAKEN	13
#define	R_PPC_GOT16		14
#define	R_PPC_GOT16_LO		15
#define	R_PPC_GOT16_HI		16
#define	R_PPC_GOT16_HA		17
#define	R_PPC_PLTREL24		18
#define	R_PPC_COPY		19
#define	R_PPC_GLOB_DAT		20
#define	R_PPC_JMP_SLOT		21
#define	R_PPC_RELATIVE		22
#define	R_PPC_LOCAL24PC		23
#define	R_PPC_UADDR32		24
#define	R_PPC_UADDR16		25
#define	R_PPC_REL32		26
#define	R_PPC_PLT32		27
#define	R_PPC_PLTREL32		28
#define	R_PPC_PLT16_LO		29
#define	R_PPC_PLT16_HI		30
#define	R_PPC_PLT16_HA		31
#define	R_PPC_SDAREL16		32
#define	R_PPC_SECTOFF		33
#define	R_PPC_SECTOFF_LO	34
#define	R_PPC_SECTOFF_HI	35
#define	R_PPC_SECTOFF_HA	36
#define	R_PPC_ADDR30		37


/* Program header */
typedef struct {
	UW	p_type;		/* segment type */
	UW	p_offset;	/* segment offset */
	VP	p_vaddr;	/* virtual address */
	VP	p_paddr;	/* physical address */
	UW	p_filesz;	/* segment size in file image */
	UW	p_memsz;	/* segment size on memory */
	UW	p_flags;	/* flags (PF_xxx) */
	UW	p_align;	/* alignment */
} Elf32_Phdr;

/* p_type */
#define	PT_NULL		0	/* invalid segment */
#define	PT_LOAD		1	/* loadable segment */
#define	PT_DYNAMIC	2	/* dynamic link information */
#define	PT_INTERP	3	/* interpreter path name */
#define	PT_NOTE		4	/* auxiliary information */
#define	PT_SHLIB	5	/* (reserved) */
#define	PT_PHDR		6	/* program header */
#define	PT_LOOS		0x60000000	/* OS specific range */


#define	PT_SUNW_UNWIND	0x6464E550	/* amd64 UNWIND program header */
#define	PT_GNU_EH_FRAM	PT_SUNW_UNWIND

#define	PT_LOWUNW	0x6FFFFFFA
#define	PT_SUNWBSS	0x6FFFFFFA	/* Sun specific segment */
#define	PT_SUNWSTACK	0x6FFFFFFB	/* describes the stack segment */
#define	PT_SUNWDTRACE	0x6FFFFFFC	/* private */
#define	PT_SUNWCAP	0x6FFFFFFD	/* hard/soft capabilities segment */
#define	PT_HISUNW	0x6FFFFFFF
#define	PT_HIOS		0x6FFFFFFF

#define	PT_LOPROC	0x70000000	/* CPU-specific */
#define	PT_HIPROC	0x7fffffff

/* p_flags */
#define	PF_R		0x4	/* Read */
#define	PF_W		0x2	/* Write */
#define	PF_X		0x1	/* Execute */
#define	PF_MASKPROC	0xf0000000	/* CPU-specific */
#define	PF_MASKOS	0x0FF00000	/* OS specific values */
#define	PF_SUNW_FAILURE	0x00100000	/* mapping absent due to failure */
#define	PN_XNUM		0xFFFF		/* extended program header index */


/* Dynamic structure */
typedef struct {
	W	d_tag;		/* type */
	union {
		UW	d_val;	/* value */
		VP	d_ptr;	/* virtual address */
	} d_un;
} Elf32_Dyn;

IMPORT	Elf32_Dyn	_DYNAMIC[];

/* d_tag  I:ignored V:d_val P:d_ptr */
#define	DT_NULL		0	/* I: _DYNAMIC array terminal mark */
#define	DT_NEEDED	1	/* V: needed library */
#define	DT_PLTRELSZ	2	/* V: PLT size */
#define	DT_PLTGOT	3	/* P: PLT and/or GOT */
#define	DT_HASH		4	/* P: symbol hash table */
#define	DT_STRTAB	5	/* P: string table */
#define	DT_SYMTAB	6	/* P: symbol table */
#define	DT_RELA		7	/* P: relocation table (Elf32_Rela) */
#define	DT_RELASZ	8	/* V: total size of DT_RELA relocation table */
#define	DT_RELAENT	9	/* V: size of DT_RELA entry */
#define	DT_STRSZ	10	/* V: total size of string table */
#define	DT_SYMENT	11	/* V: size of symbol table entry */
#define	DT_INIT		12	/* P: initialization function address */
#define	DT_FINI		13	/* P: termination function address */
#define	DT_SONAME	14	/* V: shared object name */
#define	DT_RPATH	15	/* V: library search path */
#define	DT_SYMBOLIC	16	/* I: symbol resolution algorithm switch */
#define	DT_REL		17	/* P: relocation table (Elf32_Rel) */
#define	DT_RELSZ	18	/* V: total size of DT_REL relocation table */
#define	DT_RELENT	19	/* V: size of DT_REL entry */
#define	DT_PLTREL	20	/* V: PLT type (REL/RELA) */
#define	DT_DEBUG	21	/* P: debug */
#define	DT_TEXTREL	22	/* I: text relocation */
#define	DT_JMPREL	23	/* P: PLTREL relocation entries for lazy binding */
#define	DT_LOPROC	0x70000000	/* CPU-specific */
#define	DT_HIPROC	0x7fffffff

/* For MIPS */
#define	DT_MIPS_LOCAL_GOTNO	0x7000000a  /* Number of local GOT entries */
#define	DT_MIPS_SYMTABNO	0x70000011  /* Number of DYNSYM entries */
#define	DT_MIPS_GOTSYM		0x70000013  /* First GOT entry */

/* Global offset table */
IMPORT	VP	_GLOBAL_OFFSET_TABLE_[];

/* Hashing function */
Inline UW Elf_hash( const UB *name )
{
	UW	h = 0, g;

	while ( *name != '\0' ) {
		h = (h << 4) + *name++;
		g = h & 0xf0000000;
		h = (h ^ (g >> 24)) & ~g;
	}

	return h;
}

/* Register Information */
typedef struct {
	UW	ri_gprmask;	/* generic registers used */
	UW	ri_cprmask[4];	/* coprocessor registers used */
	W	ri_gp_value;	/* gp register value */
} Elf32_RegInfo;

/* Section Group Flags (SHT_GROUP) */
#define	GRP_COMDAT	0x01

/* Hardware/Software capabilities entry */
typedef struct {
	Elf32_Word	c_tag;	/* how to interpret value */
	union {
		Elf32_Word	c_val;
		Elf32_Addr	c_ptr;
	} c_un;
} Elf32_Cap;

#define	CA_SUNW_NULL	0
#define	CA_SUNW_HW_1	1	/* first hardware capabilities entry */
#define	CA_SUNW_SF_1	2	/* first software capabilities entry */

#define	SF1_SUNW_FPKNWN	0x001	/* use/non-use of fram pointer is known. */
#define	SF1_SUNW_FPUSED	0x002	/* and fram pointer is in use */
#define	SF1_SUNW_MASK	0x003




#endif /* _T2EX_LOAD_ELF_ */
