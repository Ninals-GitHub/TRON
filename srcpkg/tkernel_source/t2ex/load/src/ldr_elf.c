/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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
 *	ldr_elf.c
 *
 *       T2EX: program load functions
 *       ELF object loader
 */

#include <basic.h>
#include <errno.h>
#include <libstr.h>
#include <tk/tkernel.h>
#include <sys/debug.h>
#include <sys/segment.h>
#include <pagedef.h>
#include "pminfo.h"
#include "source.h"
#include "elf.h"
#include <bk/memory/page.h>


#if CPU_I386
#define	LINK_ADDR	0x00100000		/* link address */
#endif

/* architecture check */
#if CPU_I386
#  define CHK_ELFDATA(n)		( (n) == ELFDATA2LSB )
#  define CHK_ELFMACH(n)		( (n) == EM_386 )
#  define SHT_RELTYPE			SHT_REL
#endif
#if CPU_SH3|CPU_SH4
#  define CHK_ELFDATA(n)		( (n) == ELFDATA2LSB )
#  define CHK_ELFMACH(n)		( (n) == EM_SH )
#  define SHT_RELTYPE			SHT_RELA
#endif
#if CPU_ARM
#  define CHK_ELFDATA(n)		( (n) == ELFDATA2LSB )
#  define CHK_ELFMACH(n)		( (n) == EM_ARM )
#  define SHT_RELTYPE			SHT_REL
#endif
#if CPU_MIPS
#  define CHK_ELFDATA(n)		( (n) == ELFDATA2LSB )
#  define CHK_ELFMACH(n)		( (n) == EM_MIPS )
#  define SHT_RELTYPE			SHT_REL
#endif
#if CPU_PPC
#  define CHK_ELFDATA(n)		( (n) == ELFDATA2MSB )
#  define CHK_ELFMACH(n)		( (n) == EM_PPC )
#  define SHT_RELTYPE			SHT_RELA
#endif

/*
 * ELF loading information
 */
typedef struct {
	B*	text_ladr;	/* text area load address */
	UW	text_fofs;	/* text area file offset */
	UW	text_size;	/* text area size */
	B*	data_ladr;	/* data area load address */
	UW	data_fofs;	/* data area file offset */
	UW	data_size;	/* data area size */
	B*	bss_ladr;	/* bss  area load address */
	UW	bss_size;	/* bss  area size */

	UH	text_shndx;	/* text area section number */
	UH	data_shndx;	/* data area section number */
	UH	bss_shndx;	/* bss  area section number */
	UW	rel_text_fofs;	/* text area relocation info file offset */
	UW	rel_text_size;	/* text area relocation info size */
	UW	rel_data_fofs;	/* data area relocation info file offset */
	UW	rel_data_size;	/* data area relocation info size */

	UW	symtbl_fofs;	/* symbol table file offset */
	UW	symtbl_size;	/* symbol table size */
	BOOL	vir_or_off;	/* relocation information type: 
				   virtual address (TRUE) or offset inside section (FALSE) */
#if CPU_MIPS
	UW	reginf_fofs;	/* register info file offset */
	UW	reginf_size;	/* register info size */
	W	gp;		/* gp register value */
#endif
#if CPU_PPC
	UW	gp_info_fofs;	/* gp info file offset */
	W	gp;		/* gp register value (_SDA_BASE_) */
#endif
} ELF_LoadInfo;


/*
 * Get ELF loading information from section header
 */
LOCAL ER GetELFLoadInfoShdr( ELF_LoadInfo *eli, Elf32_Ehdr *hdr, LoadSource *ldr )
{
	Elf32_Shdr	*shdr_buf, *shdr, *p;
	W		n;
	B		*adr;
	W		symsect = -1;
	ER		er;

	if ( hdr->e_shentsize < sizeof(Elf32_Shdr) || hdr->e_shnum == 0 )
				{ er = EX_INVAL; goto err_ret1; }

	/* Allocate buffer for reading ELF section header */
	n = hdr->e_shentsize * hdr->e_shnum;
	shdr_buf = Vmalloc(n);
	if ( shdr_buf == NULL ) { er = EX_NOEXEC; goto err_ret1; }

	/* Read section header*/
	er = ldr->read(ldr, hdr->e_shoff, shdr_buf, n);
	if ( er < E_OK ) goto err_ret2;
	if ( er < n ) { er = EX_NOEXEC; goto err_ret2; }

	for ( n = hdr->e_shnum, shdr = shdr_buf; --n >= 0;
			shdr = (Elf32_Shdr*)((B*)shdr + hdr->e_shentsize) ) {

		switch ( shdr->sh_type ) {
		  case SHT_PROGBITS:
			switch ( shdr->sh_flags ) {
			  case SHF_ALLOC|SHF_EXECINSTR:	/* text area */
				if ( eli->text_size != 0 )
					{ er = EX_NOEXEC; goto err_ret2; }
				eli->text_ladr = shdr->sh_addr;
				eli->text_fofs = shdr->sh_offset;
				eli->text_size = shdr->sh_size;
				eli->text_shndx = shdr - shdr_buf;
				break;
			  case SHF_ALLOC|SHF_WRITE:	/* data area */
				if ( eli->data_ladr != NULL )
					{ er = EX_NOEXEC; goto err_ret2; }
				eli->data_ladr = shdr->sh_addr;
				eli->data_fofs = shdr->sh_offset;
				eli->data_size = shdr->sh_size;
				eli->data_shndx = shdr - shdr_buf;
				break;
#if CPU_PPC
			  case SHF_WRITE:		/* gp_info */
				if ( shdr->sh_size != 4 ) break;
				if ( eli->gp_info_fofs != 0 )
					{ er = EX_NOEXEC; goto err_ret2; }
				eli->gp_info_fofs = shdr->sh_offset;
				break;
#endif
			  default:
				if ( (shdr->sh_flags & SHF_ALLOC) == 0 )
							continue; /* ignore */
				er = EX_NOEXEC; goto err_ret2;
			}
			break;

		  case SHT_NOBITS:
			switch ( shdr->sh_flags ) {
			  case SHF_ALLOC|SHF_WRITE:	/* bss area */
				if ( eli->bss_ladr != NULL )
					{ er = EX_NOEXEC; goto err_ret2; }
				eli->bss_ladr = shdr->sh_addr;
				eli->bss_size = shdr->sh_size;
				eli->bss_shndx = shdr - shdr_buf;
				break;
			  case 0:
				continue; /* ignore */
			  default:
				er = EX_NOEXEC; goto err_ret2;
			}
			break;

		  case SHT_RELTYPE:
			if ( shdr->sh_info >= hdr->e_shnum
			  || shdr->sh_link >= hdr->e_shnum )
					{ er = EX_NOEXEC; goto err_ret2; }

			/* relocation target */
			p = &shdr_buf[shdr->sh_info];
			if ( p->sh_type != SHT_PROGBITS )
				continue;

			switch ( p->sh_flags ) {
			  case SHF_ALLOC|SHF_EXECINSTR:	/* text area */
				if ( eli->rel_text_size != 0 )
					{ er = EX_NOEXEC; goto err_ret2; }
				eli->rel_text_fofs = shdr->sh_offset;
				eli->rel_text_size = shdr->sh_size;
				break;
			  case SHF_ALLOC|SHF_WRITE:	/* data area */
				if ( eli->rel_data_size != 0 )
					{ er = EX_NOEXEC; goto err_ret2; }
				eli->rel_data_fofs = shdr->sh_offset;
				eli->rel_data_size = shdr->sh_size;
				break;
			  case 0:
				continue; /* ignore */
			  default:
				er = EX_NOEXEC; goto err_ret2;
			}

			if ( symsect >= 0 && shdr->sh_link != (UW)symsect )
					{ er = EX_NOEXEC; goto err_ret2; }
			symsect = shdr->sh_link;
			break;
#if CPU_MIPS
		  case SHT_MIPS_REGINFO:
	  		if( eli->reginf_size != 0 )
	  			{ er = EX_NOEXEC; goto err_ret2; }
			eli->reginf_fofs = shdr->sh_offset;
			eli->reginf_size = shdr->sh_size;
		  	break;
#endif
		  default:
			continue; /* ignore */
		}
	}

	/* Check for loading information validity */
	if ( eli->text_size == 0 ) { er = EX_NOEXEC; goto err_ret2; }
	adr = PageAlignU(eli->text_ladr + eli->text_size);
	if ( eli->data_ladr != adr ) { er = EX_NOEXEC; goto err_ret2; }
	adr += eli->data_size;
	if ( eli->bss_ladr < adr ) { er = EX_NOEXEC; goto err_ret2; }

	if ( symsect >= 0 ) {
		/* symbol table for relocation */
		if ( shdr_buf[symsect].sh_type != SHT_SYMTAB )
					{ er = EX_NOEXEC; goto err_ret2; }
		eli->symtbl_fofs = shdr_buf[symsect].sh_offset;
		eli->symtbl_size = shdr_buf[symsect].sh_size;
	}

	Vfree(shdr_buf);

	return E_OK;

err_ret2:
	Vfree(shdr_buf);
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("GetELFLoadInfoShdr ercd = %d\n", er));
#endif
	return er;
}

/*
 * Check if segments with dynamic information exist
 */
LOCAL BOOL HasDynSeg( Elf32_Ehdr *hdr, LoadSource *ldr )
{
	Elf32_Phdr	*phdr_buf, *phdr;
	W		n;
	ER		er;

	if ( hdr->e_phentsize < sizeof(Elf32_Phdr) || hdr->e_phnum < 1 )
				{ er = EX_NOEXEC; goto err_ret1; }

	/* Allocate buffer for reading program header */
	n = hdr->e_phentsize * hdr->e_phnum;
	phdr_buf = Vmalloc(n);
	if ( phdr_buf == NULL ) { er = EX_NOMEM; goto err_ret1; }

	/* Read program header */
	er = ldr->read(ldr, hdr->e_phoff, phdr_buf, n);
	if ( er < E_OK ) goto err_ret2;
	if ( er < n ) { er = EX_NOEXEC; goto err_ret2; }

	for ( n = hdr->e_phnum, phdr = phdr_buf; --n >= 0;
			phdr = (Elf32_Phdr*)((B*)phdr + hdr->e_phentsize) ) {

		if ( phdr->p_type == PT_DYNAMIC )
			goto found;
	}

	Vfree(phdr_buf);
	return FALSE;

found:
	Vfree(phdr_buf);
	return TRUE;

err_ret2:
	Vfree(phdr_buf);
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("HasDynSeg ercd = %d\n", er));
#endif
	return FALSE;
}

/*
 * Check if sections with relocation information exist
 */
LOCAL BOOL HasRelSec( Elf32_Ehdr *hdr, LoadSource *ldr )
{
	Elf32_Shdr	*shdr_buf, *shdr;
	W		n;
	ER		er;

	if ( hdr->e_shentsize < sizeof(Elf32_Shdr) || hdr->e_shnum == 0 )
				{ er = EX_NOEXEC; goto err_ret1; }

	/* Allocate buffer for reading section header */
	n = hdr->e_shentsize * hdr->e_shnum;
	shdr_buf = Vmalloc(n);
	if ( shdr_buf == NULL ) { er = EX_NOMEM; goto err_ret1; }

	/* Read section header */
	er = ldr->read(ldr, hdr->e_shoff, shdr_buf, n);
	if ( er < E_OK ) goto err_ret2;
	if ( er < n ) { er = EX_NOEXEC; goto err_ret2; }

	for ( n = hdr->e_shnum, shdr = shdr_buf; --n >= 0;
			shdr = (Elf32_Shdr*)((B*)shdr + hdr->e_shentsize) ) {

		switch ( shdr->sh_type ) {
		  case SHT_RELTYPE:
			goto found;
		  default:
			continue; /* ignore */
		}
	}
	Vfree(shdr_buf);
	return FALSE;

found:
	Vfree(shdr_buf);
	return TRUE;

err_ret2:
	Vfree(shdr_buf);
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("HasRelSec ercd = %d\n", er));
#endif
	return FALSE;
}

/*
 * Get ELF loading information from program header
 */
LOCAL ER GetELFLoadInfoPhdr( ELF_LoadInfo *eli, Elf32_Ehdr *hdr, LoadSource *ldr )
{
	Elf32_Phdr	*phdr_buf, *phdr;
	W		n;
	B		*adr;
	ER		er;

	if ( hdr->e_phentsize < sizeof(Elf32_Phdr) || hdr->e_phnum < 2 )
				{ er = EX_NOEXEC; goto err_ret1; }

	/* Allocate buffer for reading program header */
	n = hdr->e_phentsize * hdr->e_phnum;
	phdr_buf = Vmalloc(n);
	if ( phdr_buf == NULL ) { er = EX_NOMEM; goto err_ret1; }

	/* Read program header */
	er = ldr->read(ldr, hdr->e_phoff, phdr_buf, n);
	if ( er < E_OK ) goto err_ret2;
	if ( er < n ) { er = EX_NOEXEC; goto err_ret2; }

	for ( n = hdr->e_phnum, phdr = phdr_buf; --n >= 0;
			phdr = (Elf32_Phdr*)((B*)phdr + hdr->e_phentsize) ) {

		/* Ignore except for loadable segments */
		if ( phdr->p_type != PT_LOAD ) continue;

		switch ( phdr->p_flags ) {
		  case PF_R|PF_X:	/* text area */
			if ( eli->text_size != 0 )
					{ er = EX_NOEXEC; goto err_ret2; }
			eli->text_ladr = phdr->p_vaddr;
			eli->text_fofs = phdr->p_offset;
			eli->text_size = phdr->p_filesz;
			break;

		  case PF_R|PF_W:	/* data or bss area */
		  case PF_R|PF_W|PF_X:
			if ( eli->data_ladr != NULL )
					{ er = EX_NOEXEC; goto err_ret2; }
			eli->data_ladr = phdr->p_vaddr;
			eli->data_fofs = phdr->p_offset;
			eli->data_size = phdr->p_filesz;
			eli->bss_ladr = (B*)phdr->p_vaddr + phdr->p_filesz;
			eli->bss_size = phdr->p_memsz - phdr->p_filesz;
			break;

		  case 0:
			if ( phdr->p_filesz > 0 || phdr->p_memsz > 0 )
					{ er = EX_NOEXEC; goto err_ret2; }
			continue; /* ignore */

		  default:
			er = EX_NOEXEC; goto err_ret2;
		}
	}

	/* Check for loading information validity */
	if ( eli->text_size == 0 || eli->data_ladr == NULL )
				{ er = EX_NOEXEC; goto err_ret2; }
	adr = eli->text_ladr + eli->text_size;
	if ( eli->data_ladr < (B*)PageAlignU(adr) )
				{ er = EX_NOEXEC; goto err_ret2; }

	Vfree(phdr_buf);

	return E_OK;

err_ret2:
	Vfree(phdr_buf);
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("GetELFLoadInfoPhdr ercd = %d\n", er));
#endif
	return er;
}

/*
 * Get ELF loading information
 */
LOCAL ER GetELFLoadInfo( ELF_LoadInfo *eli, Elf32_Ehdr *hdr, LoadSource *ldr, BOOL relsec )
{
	ER	er;

	memset(eli, 0, sizeof(ELF_LoadInfo));

	/* Check if header is valid */
	if ( !(hdr->e_ident[EI_CLASS] == ELFCLASS32
	    && CHK_ELFDATA(hdr->e_ident[EI_DATA])
	    && hdr->e_ident[EI_VERSION] == 1
	    && CHK_ELFMACH(hdr->e_machine)
	    && hdr->e_version == EV_CURRENT) ) {
		er = EX_NOEXEC; goto err_ret;
	}

	switch ( hdr->e_type ) {
	  case ET_EXEC:
#if 0
	  	if ( relsec ) {
	  		printf("GetELFLoadInfoShdr\n");
			/* Get loading information from section header */
			eli->vir_or_off = TRUE;
			er = GetELFLoadInfoShdr(eli, hdr, ldr);
		} else {
			printf("GetELFLoadInfoPhdr\n");
			/* Get loading information from program header */
			er = GetELFLoadInfoPhdr(eli, hdr, ldr);
		}
#endif
		er = GetELFLoadInfoPhdr(eli, hdr, ldr);
			if ( er < E_OK ) {goto err_ret;}
		break;

	  case ET_DYN:
		/* Get loading information from program header */
		er = GetELFLoadInfoPhdr(eli, hdr, ldr);
		if ( er < E_OK ) {goto err_ret;}
		break;

	  case ET_REL:
		/* Get loading information from section header */
		er = GetELFLoadInfoShdr(eli, hdr, ldr);
		if ( er < E_OK ) {goto err_ret;}
		break;

	  default:
		er = EX_NOEXEC; goto err_ret;
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("GetELFLoadInfo ercd = %d\n", er));
#endif
	return er;
}

#if CPU_MIPS
typedef struct {
	W	*addr;
	UW	val;
} Ahi_Info;
#define AHI_INFO_UNIT	1
#endif

/*
 * Relocate a single section
 */
LOCAL ER reloc1( ELF_LoadInfo *eli, LoadSource *ldr, W rtbl, W rtblsz, W lofs,
		B *sect, W sectsz, Elf32_Sym *symtbl, W syment )
{
#if SHT_RELTYPE == SHT_REL
	Elf32_Rel	*reloc, *rp, *reloc_end;
#endif
#if SHT_RELTYPE == SHT_RELA
	Elf32_Rela	*reloc, *rp, *reloc_end;
#endif
#if CPU_ARM
	W		ofs;
#endif
#if CPU_MIPS
	Ahi_Info	*ahi_info = NULL;
	Ahi_Info	*ahi_infotmp;
	W		ahi_infomax = AHI_INFO_UNIT;
	W		ahi_infonext = 0;
	W		ahi_infoused = 0;
	W		ind;
	W		ahl, addend, nadr;
	UH		*hp;
#endif
	W		*wp, sv, i;
	B		*ladr;
	ER		er;

	if ( rtblsz == 0 ) return E_OK;  /* No relocation target */

	/* Read relocation information */
	reloc = Vmalloc(rtblsz);
	if ( reloc == NULL ) { er = EX_NOMEM; goto err_ret1; }
	er = ldr->read(ldr, rtbl, reloc, rtblsz);
	if ( er < E_OK ) goto err_ret2;
	if ( er < rtblsz ) { er = EX_NOEXEC; goto err_ret2; }

	reloc_end = &reloc[rtblsz / sizeof(*reloc)];
	if ( eli->vir_or_off ) {
		for ( rp = reloc; rp < reloc_end; rp++ ) {
			/* Convert to offset inside section */
			rp->r_offset -= ((UW)sect - lofs);
		}
	}

	for ( rp = reloc; rp < reloc_end; rp++ ) {

		i = ELF32_R_SYM(rp->r_info);
		if ( i >= syment || (UW)rp->r_offset >= (UW)sectsz )
					{ er = EX_NOEXEC; goto err_ret2; }

		/* Skip unless relocation is needed */
		if ( ELF32_R_TYPE(rp->r_info) == 0 ) continue;

		/* Get relocation target and its symbol value */
		wp = (W*)(sect + (UW)rp->r_offset);
		sv = (W)symtbl[i].st_value;

		/* Get section to which the symbol belongs */
		if ( symtbl[i].st_shndx == eli->text_shndx ) {
			ladr = eli->text_ladr;
		} else if ( symtbl[i].st_shndx == eli->data_shndx ) {
			ladr = eli->data_ladr;
		} else if ( symtbl[i].st_shndx == eli->bss_shndx ) {
			ladr = eli->bss_ladr;
		} else if ( symtbl[i].st_shndx == SHN_ABS ) {
			ladr = (B*)0 - lofs;
		} else if ( symtbl[i].st_shndx == SHN_UNDEF ) {
			if ( ELF32_ST_BIND(symtbl[i].st_info) == STB_WEAK )
								continue;
#if 1
		/*
		 * GNU strip seems to have a bug that makes symbol numbers 
		 * of relocation information to 0, when symbols having 
		 * the value 0 are referred to. Such phonomenon is observed 
		 * on SH, ARM, MIPS, while other architecture might be 
		 * affected as well.
		 * As a countermeasure, here we treat symbols having 
		 * number 0 to have the value of 0. However, the lack of 
		 * correct symbol number results in the lack of knowledge 
		 * of the section the symbol belongs to. For this reason, 
		 * it is assumed that only ABS section contains symbols with 
		 * value 0. Hence, it is not allowed to locate symbols at 
		 * address 0 at link time. 
		 */
			if ( i == 0 ) {
				ladr = (B*)0 - lofs; /* ABS section */
				sv = 0;
			} else
#endif
			{ er = EX_NOEXEC; goto err_ret2; }
		} else {
			er = EX_NOEXEC; goto err_ret2;
		}

#if CPU_I386
		/* 
		 * GNU strip 2.8.1 seems to have a bug that converts 
		 * st_value to a corresponding logical address, 
		 * where section offset value should instead be given 
		 * according to the ELF specification. 
		 * In order to cope with it, st_value is converted back to 
		 * a section offset when st_value >= LINK_ADDR (0x00100000), 
		 * as the logical address starts at LINK_ADDR as specified 
		 * in linker script.
		 */
		if ( sv >= LINK_ADDR ) sv -= (W)ladr;
#endif

		/* Execute relocation */
		switch ( ELF32_R_TYPE(rp->r_info) ) {
#if CPU_I386
		  case R_386_32:
			if ( eli->vir_or_off ) {
				sv = 0;
				*wp -= (W)ladr;
			}
			*wp += sv + (W)ladr + lofs;
			break;

		  case R_386_PC32:
			if ( eli->vir_or_off ) {
				break;
			}
			*wp += sv - (W)rp->r_offset;
			break;
#endif
#if CPU_ARM
		  case R_ARM_PC24:
			if ( eli->vir_or_off ) {
				break;
			}
			ofs = (W)((UW)*wp << 8) >> 8; /* sign expansion of 24-bit integer */
			if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){
				ofs -= (W)rp->r_offset >> 2;
			} else {
				ofs += (sv - (W)rp->r_offset) >> 2;
			}
			*wp = (*wp & 0xff000000) | (ofs & 0x00ffffff);
			break;

		  case R_ARM_ABS32:
			if ( eli->vir_or_off ) {
				sv = 0;
				*wp -= (W)ladr;
			}
			*wp += sv + (W)ladr + lofs;
			break;
		  case R_ARM_THM_PC22:
			if ( !eli->vir_or_off ) {
				er = EX_NOEXEC; goto err_ret2;
			}
			break;
#endif
#if CPU_MIPS
		  case R_MIPS_32:
			if ( eli->vir_or_off ) {
				SetMisalignW((UB*)wp, GetMisalignW((UB*)wp) - sv);
				sv -= (W)ladr;
			}
			SetMisalignW((UB*)wp, GetMisalignW((UB*)wp) + (W)ladr + sv + lofs);
		  	break;

		  case R_MIPS_26:
			if ( eli->vir_or_off ) {
				if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){	
					nadr = (((*wp & 0x03ffffff) << 2) - (W)ladr) >> 2;
					*wp = ( *wp & 0xfc000000) | ( nadr & 0x03ffffff);
				} else {
					*wp &= 0xfc000000;
				}
				sv -= (W)ladr;
			}
			addend = *wp & 0x03ffffff;
			if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){	
				nadr = (addend << 2) | (((W)ladr + (W)rp->r_offset) & 0xf0000000);
			}else{
				nadr = (W)((UW)addend << 6) >> 4; 
			}
			nadr += (W)ladr + sv + lofs;
			nadr >>= 2;
			*wp = ( *wp & 0xfc000000) | ( nadr & 0x03ffffff);
		  	break;

		  case R_MIPS_HI16:
			if ( ahi_info == NULL) {
				ahi_info = Vmalloc( sizeof( Ahi_Info) * AHI_INFO_UNIT);
				if ( ahi_info == NULL) { er = EX_NOMEM; goto err_ret2; }
			}
			if( ahi_infoused ) {
				ahi_infonext = 0;
				ahi_infoused = 0;
			}
			if( ahi_infomax <= ahi_infonext){
				ahi_infotmp = Vrealloc( ahi_info, sizeof( Ahi_Info) * ( ahi_infomax + AHI_INFO_UNIT));
				if ( ahi_infotmp == NULL ) { er = EX_NOMEM; goto err_ret2; }
				ahi_info = ahi_infotmp;
				ahi_infomax += AHI_INFO_UNIT;
			}
			ahi_info[ ahi_infonext].addr = wp;
			ahi_info[ ahi_infonext].val  = (UW)*wp << 16;
			ahi_infonext++;
		  	break;

		  case R_MIPS_LO16:
			if ( ahi_info == NULL) { err = EX_NOEXEC; goto err_ret2; }
			if ( ahi_infoused == 0 ) {
				ahi_infoused = 1;
			  	for ( ind = 0; ind < ahi_infonext; ind++ ) {
				  	ahl = ( ahi_info[ ind].val & 0xffff0000) + ( short)(*wp & 0xffff);
					if ( eli->vir_or_off ) {
						ahl -= sv;
						sv -= (W)ladr;
					}
					nadr = ahl + (W)ladr + sv + lofs;
			  		*ahi_info[ ind].addr = ((( nadr - (short)nadr) >> 16) & 0x0000ffff) 
			  					| ( *ahi_info[ ind].addr & 0xffff0000);
				}
				if ( 1 < ahi_infonext ) {
					ahi_info[ 0] = ahi_info[ ahi_infonext - 1];
					ahi_infonext = 1;
				}
			} else {
			  	ahl = ( ahi_info[ 0].val & 0xffff0000) + ( short)(*wp & 0xffff);
				if ( eli->vir_or_off ) {
					ahl -= sv;
					sv -= (W)ladr;
				}
				nadr = ahl + (W)ladr + sv + lofs;
			}
		  	*wp = ( nadr & 0xffff) | (*wp & 0xffff0000);
		  	break;

		  case R_MIPS_GPREL16:
		  case R_MIPS_LITERAL:
		  	if ( eli->vir_or_off ) {
				break;
		  	}
			addend = *wp & 0xffff;
			if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){
				nadr = ((W)ladr + (( sv + addend + eli->gp ) & 0x0000ffff) - eli->gp) & 0x0000ffff;
			}else{
				nadr = ((W)ladr + sv + addend - eli->gp ) & 0x0000ffff;
			}
		  	*wp = nadr | (*wp & 0xffff0000);
		  	break;

		  case R_MIPS16_26:
		  	if ( !eli->vir_or_off ) {
				er = EX_NOEXEC; goto err_ret2;
		  	}

			hp = (UH*)wp;
			addend =  ((UW)(*hp & 0x03e0) << 13)
				| ((UW)(*hp & 0x001f) << 23)
				| ((UW)*(hp + 1) << 2);
			if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){	
				addend -= (W)ladr;
			} else {
				addend = 0;
			}
			sv -= (W)ladr;

			/***/
			if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){	
				nadr = (addend << 2) | (((W)ladr + (W)rp->r_offset) & 0xf0000000);
			}else{
				nadr = (W)((UW)addend << 6) >> 4; 
			}
			nadr += (W)ladr + sv + lofs;
			nadr >>= 2;

			*hp = (*hp & 0xfc00) 
				| (( nadr & 0x03e00000) >> 21)
				| (( nadr & 0x001f0000) >> 11);
		  	*(hp+1) = nadr & 0xffff;
		  	break;

		  case	R_MIPS16_GPREL:
		  	if ( !eli->vir_or_off ) {
				er = EX_NOEXEC; goto err_ret2;
		  	}
		  	break;
		  
#endif
#if CPU_SH3|CPU_SH4
		  case R_SH_DIR32:
			if ( ELF32_ST_TYPE(symtbl[i].st_info) == STT_SECTION ){
				*wp += rp->r_addend + lofs;
			} else {
				if ( eli->vir_or_off ) {
					*wp -= sv;
					sv -= (W)ladr;
				}
				*wp += sv + rp->r_addend + (W)ladr + lofs;
			}
			break;
		  case R_SH_REL32:
			if ( !eli->vir_or_off ) {
				er = EX_NOEXEC; goto err_ret2;
			}
		  	break;
#endif
#if CPU_PPC
		  case R_PPC_ADDR32:
			if ( !eli->vir_or_off ) sv += (W)ladr;
			*wp = sv + rp->r_addend + lofs;
			break;
		  case R_PPC_ADDR16_LO:
			if ( !eli->vir_or_off ) sv += (W)ladr;
			*(H*)wp = sv + rp->r_addend + lofs;
			break;
		  case R_PPC_ADDR16_HI:
			if ( !eli->vir_or_off ) sv += (W)ladr;
			*(H*)wp = (sv + rp->r_addend + lofs) >> 16;
			break;
		  case R_PPC_ADDR16_HA:
			if ( !eli->vir_or_off ) sv += (W)ladr;
			sv += rp->r_addend + lofs;
			*(H*)wp = (sv >> 16) - ((H)sv >> 15);
			break;
		  case R_PPC_REL32:
			if ( eli->vir_or_off ) break;
			*wp = sv + rp->r_addend - (W)rp->r_offset;
			break;
		  case R_PPC_REL24:
		  case R_PPC_PLTREL24:	/* direct jump (not using PLT) */
			if ( eli->vir_or_off ) break;
			sv += rp->r_addend - (W)rp->r_offset;
			if ( sv < -0x02000000 || sv > 0x01fffffc )
				{ er = EX_NOEXEC; goto err_ret2; }
			*wp = (*wp & 0xfc000003) | (sv & 0x03fffffc);
			break;
		  case R_PPC_REL14:
		  case R_PPC_REL14_BRTAKEN:
		  case R_PPC_REL14_BRNTAKEN:
			if ( eli->vir_or_off ) break;
			sv += rp->r_addend - (W)rp->r_offset;
			if ( sv < -0x00008000 || sv > 0x00007ffc )
				{ er = EX_NOEXEC; goto err_ret2; }
			*wp = (*wp & 0xffff0003) | (sv & 0x0000fffc);
			switch ( ELF32_R_TYPE(rp->r_info) ) {
			  case R_PPC_REL14_BRTAKEN:
				if ( sv >= 0 )	*wp |=  0x00200000;
				else		*wp &= ~0x00200000;
				break;
			  case R_PPC_REL14_BRNTAKEN:
				if ( sv >= 0 )	*wp &= ~0x00200000;
				else		*wp |=  0x00200000;
				break;
			}
			break;
		  case R_PPC_SDAREL16:
			if ( eli->vir_or_off ) break;
			sv += rp->r_addend + (W)ladr - eli->gp;
			if ( sv < -0x00008000 || sv > 0x00007fff )
				{ er = EX_NOEXEC; goto err_ret2; }
			*(H*)wp = sv;
			break;
#endif
		  default:
			er = EX_NOEXEC; goto err_ret2;
		}
	}

#if CPU_MIPS
	if( ahi_info ) { 
		Vfree(ahi_info);
	}
#endif
	Vfree(reloc);

	return E_OK;

err_ret2:
#if CPU_MIPS
	if( ahi_info ) {
		Vfree(ahi_info);
	}
#endif
	Vfree(reloc);
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("reloc1 ercd = %d\n", er));
#endif
	return er;
}

#if CPU_MIPS
/*
 * Get gp
 */
LOCAL ER elf_get_gp( LoadSource *ldr, ELF_LoadInfo *eli )
{
	ER	er;
	W	n;
	Elf32_RegInfo	reginfo;

	/* Read register information */
	n = eli->reginf_size;
	if( n <= 0 ) { er = EX_NOEXEC; goto err_ret; }

	er = ldr->read(ldr, eli->reginf_fofs, &reginfo, n);
	if ( er < E_OK ) goto err_ret;
	if ( er < n ) { er = EX_NOEXEC; goto err_ret; }

	eli->gp = reginfo.ri_gp_value;
	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("elf_get_gp ercd = %d\n", er));
#endif
	return er;
}
#endif

#if CPU_PPC
/*
 * Get gp
 */
LOCAL ER elf_get_gp( LoadSource* ldr, ELF_LoadInfo *eli )
{
	UW	gp_info;
	ER	er;

	if ( eli->gp_info_fofs == 0 ) { er = EX_NOEXEC; goto err_ret; }

	/* Read gp information */
	er = ldr->read(ldr, eli->gp_info_fofs, (B*)&gp_info, sizeof(gp_info));
	if ( er < E_OK ) goto err_ret;
	if ( er < sizeof(gp_info) ) { er = EX_NOEXEC; goto err_ret; }

	eli->gp = gp_info;

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("elf_get_gp ercd = %d \n", er));
#endif
	return er;
}
#endif

/*
 * ELF relocation
 */
LOCAL ER elf_relocation( ProgInfo *pg, LoadSource *ldr, ELF_LoadInfo *eli, W lofs )
{
	Elf32_Sym	*symtbl;
	W		n;
	ER		er;

	if ( eli->symtbl_size == 0 )
			return E_OK;  /* no relocation information */

	/* Read symbol table */
	n = eli->symtbl_size;
	symtbl = Vmalloc(n);
	if ( symtbl == NULL ) { er = EX_NOMEM; goto err_ret1; }
	er = ldr->read(ldr, eli->symtbl_fofs, symtbl, n);
	if ( er < E_OK ) goto err_ret2;
	if ( er < n ) { er = EX_NOEXEC; goto err_ret2; }

	n /= sizeof(Elf32_Sym);

#if CPU_MIPS|CPU_PPC
	er = elf_get_gp(ldr, eli); /* get gp */
	if ( er < E_OK ) goto err_ret2;
#endif

	/* text area relocation */
	er = reloc1(eli, ldr, eli->rel_text_fofs, eli->rel_text_size, lofs,
			eli->text_ladr + lofs, eli->text_size, symtbl, n);
	if ( er < E_OK ) goto err_ret2;

	/* data area relocation */
	er = reloc1(eli, ldr, eli->rel_data_fofs, eli->rel_data_size, lofs,
			eli->data_ladr + lofs, eli->data_size, symtbl, n);
	if ( er < E_OK ) goto err_ret2;

	Vfree(symtbl);

	return E_OK;

err_ret2:
	Vfree(symtbl);
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("elf_relocation ercd = %d\n", er));
#endif
	return er;
}

/*
 * Load ELF object
 */
EXPORT ER elf_load( ProgInfo *pg, LoadSource *ldr, UINT attr, Elf32_Ehdr *hdr )
{
	ELF_LoadInfo	eli;
	B		*ladr, *top_adr;
	UW		npage;
	W		lofs, sz;
	BOOL		relsec;
	ER		er;

	if ( hdr->e_type == ET_EXEC ) {
		relsec = (HasDynSeg(hdr, ldr) ? FALSE : HasRelSec(hdr, ldr));
	} else {
		relsec = FALSE;
	}

	/* Dynamic link library is not allowed as program module */
	if ( hdr->e_type == ET_DYN )
			{ printf("elf_load 01\n");er = EX_NOEXEC; goto err_ret1; }

	/* Get ELF loading information */
	er = GetELFLoadInfo(&eli, hdr, ldr, relsec);
	if ( er < E_OK ) {printf("elf_load 02\n");goto err_ret1;}

	top_adr = PageAlignL(eli.text_ladr);

	/* Get load size (the number of pages) */
	npage = PageCount(eli.bss_ladr + eli.bss_size - top_adr);

	ladr = top_adr;
	if ( hdr->e_type != ET_EXEC
		|| (hdr->e_type == ET_EXEC && eli.vir_or_off) ) {

		/* Allocate memory for loading program */
		er = tk_get_smb((void**)&ladr, npage, attr);
		if ( er < E_OK ) { printf("elf_load 03\n");er = EX_NOMEM; goto err_ret1; }
	}

	pg->loadadr = ladr; /* load address */
	pg->loadsz = eli.bss_ladr + eli.bss_size - top_adr; /* program size (including bss) */

	lofs = ladr - top_adr;
	
#ifdef _BTRON_
	/* -------------------------------------------------------------------- */
	/* free virtual memory and page tables for user space			*/
	/* -------------------------------------------------------------------- */
	free_vm(get_current());
	
#endif

	/* Read text area */
	printf("ldr:0x%08X eli.text_fofs:0x%08X eli.text_ladr + lofs:0x%08X eli.text_size:0x%08X\n",ldr, eli.text_fofs, eli.text_ladr + lofs, eli.text_size);
	er = ldr->read(ldr, eli.text_fofs, eli.text_ladr + lofs, eli.text_size);
	if ( er < E_OK ) {printf("elf_load 04\n");goto err_ret2;}
	if ( er < eli.text_size ) { er = EX_NOEXEC; goto err_ret2; }

	/* Read data area */
	er = ldr->read(ldr, eli.data_fofs, eli.data_ladr + lofs, eli.data_size);
	if ( er < E_OK ) {printf("elf_load 05\n");goto err_ret2;}
	if ( er < eli.data_size ) { er = EX_NOEXEC; goto err_ret2; }

	if ( hdr->e_type == ET_REL
			|| ( hdr->e_type == ET_EXEC && eli.vir_or_off) ) {
		/* Relocation */
		er = elf_relocation(pg, ldr, &eli, lofs);
				if ( er < E_OK ) {printf("elf_load 06\n");goto err_ret2;}
	}

	/* Make text area executable, and non-writable */
	sz = eli.text_ladr + eli.text_size - top_adr;
	er = SetMemoryAccess(ladr, sz, MM_READ|MM_EXECUTE);
	if ( er < 0 ) {printf("elf_load 07\n");goto err_ret2;}

	/* Flush memory cache
		data area is flushed as well, as it may be executed on some platforms(PowerPC) */
	FlushMemCache(ladr, sz, TCM_ICACHE|TCM_DCACHE);
	FlushMemCache(eli.data_ladr + lofs, eli.data_size, TCM_ICACHE|TCM_DCACHE);

	/* Get entry point address */
	pg->entry = (FP)((B*)hdr->e_entry + lofs);
	pg->modentry = NULL;

	return E_OK;

err_ret2:
	if ( hdr->e_type != ET_EXEC
		|| ( hdr->e_type == ET_EXEC && eli.vir_or_off) ) {
		tk_rel_smb(ladr);
	}
err_ret1:
#ifdef DEBUG
	TM_DEBUG_PRINT(("elf_load ercd = %d\n", er));
#endif
	return er;
}
