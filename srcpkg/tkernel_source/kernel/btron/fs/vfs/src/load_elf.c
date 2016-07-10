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

#include <bk/kernel.h>
#include <bk/elf.h>
#include <bk/sys.h>
#include <bk/fs/vfs.h>
#include <bk/fs/load_elf.h>
#include <bk/fs/vdso.h>
#include <bk/memory/page.h>
#include <bk/memory/vm.h>
#include <bk/memory/mmap.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct dynamic_info;

LOCAL int read_elf32(int fd, struct elf_info *elf_info);
LOCAL char* read_interp_name32(int fd, struct elf_info *elf_info);
LOCAL int
load_elf32(int fd, struct elf_info *elf_info, struct vdso_load_info *vdso_info);
LOCAL int relocate_elf32(unsigned long load_addr, unsigned long elf_addr);

LOCAL ssize_t read_elf32_header(int fd, Elf32_Ehdr *buf);
LOCAL int check_elf32_header(const Elf32_Ehdr *header);
LOCAL int read_elf32_phdr(int fd, struct elf_info *elf_info);
LOCAL int read_elf32_shdr(int fd, struct elf_info *elf_info);
LOCAL int has_pt_interp(struct elf_info *elf_info);
LOCAL void show_elf32_header(Elf32_Ehdr *ehdr);
LOCAL INLINE Elf32_Phdr* get_first_phdr(Elf32_Ehdr *hdr);
LOCAL INLINE Elf32_Shdr* get_first_shdr(Elf32_Ehdr *hdr);
LOCAL Elf32_Phdr* get_dynamic_segment(Elf32_Ehdr *hdr);
LOCAL int
read_dynamic_section(long offset, Elf32_Phdr *dyn_phdr,
			Elf32_Dyn *dyn_entry, struct dynamic_info *dynamic_info);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
struct dynamic_info {
	char *needed;				// DT_NEEDED
	unsigned long *hash;			// DT_HASH
	
	struct {
		char *str;			// DT_STRTAB
		unsigned long strsz;		// DT_STRSZ
	} strtab;
	
	struct {
		Elf32_Sym *entry;		// DT_SYMTAB
		unsigned long syment;		// DT_SYMENT
	} symtab;
	
	struct {
		Elf32_Rela *entry;		// DT_RELA
		unsigned long relasz;		// DT_RELASZ
		unsigned long relaent;		// DT_RELAENT
	} relatab;
	
	struct {
		unsigned long init;		// DT_INIT
		unsigned long fini;		// DT_FINI
	} func;
	
	char *soname;				// DT_SONAME
	char *rpath;				// DT_RPATH;
	
	unsigned long textrel;			// DT_TEXTREL
	
	struct {
		unsigned long jmprel;		// DT_JMPREL
		unsigned long pltrelsz;		// DT_PLTRELSZ
		unsigned long pltrel;		// DT_PLTREL
	} jmp;
};

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
 Funtion	:init_elf_info
 Input		:struct elf_info *elf_info
 		 < elf information to initialize >
 Output		:struct elf_info *elf_info
 		 < elf information to initialize >
 Return		:void
 Description	:initialize a elf information 
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void init_elf_info(struct elf_info *elf_info)
{
	elf_info->shdr = NULL;
	elf_info->phdr = NULL;
	elf_info->interp_index = 0;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_elf_info
 Input		:struct elf_info *elf_info
 		 < elf information to free >
 Output		:void
 Return		:void
 Description	:free memories for elf inforamtion
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_elf_info(struct elf_info *elf_info)
{
	if (elf_info->phdr) {
		kfree(elf_info->phdr);
	}
	
	if (elf_info->shdr) {
		kfree(elf_info->shdr);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read_elf
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < read elf information >
 Output		:struct elf_info *elf_info
 		 < read elf information >
 Return		:int
 		 < result >
 Description	:read elf header
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int read_elf(int fd, struct elf_info *elf_info)
{
	return(read_elf32(fd, elf_info));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read_interp_name
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:void
 Return		:char*
 		 < interp name >
 Description	:read intepreter name from a segment
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT char* read_interp_name(int fd, struct elf_info *elf_info)
{
	return(read_interp_name32(fd, elf_info));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:load_elf
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:void
 Return		:int
 		 < result >
 Description	:load executable file into user memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int load_elf(int fd, struct elf_info *elf_info)
{
	return(load_elf32(fd, elf_info, NULL));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:load_vdso_elf
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 		 struct vdso_load_info *vdso_load_info
 		 < vdso load information >
 Output		:struct vdso_load_info *vdso_load_info
 		 < vdso load information >
 Return		:int
 		 < result >
 Description	:load executable file into user memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int load_vdso_elf(int fd, struct elf_info *elf_info,
				struct vdso_load_info *vdso_load_info)
{
	return(load_elf32(fd, elf_info, vdso_load_info));
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
----------------------------------------------------------------------------------
	elf32 functions
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:read_elf32
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < read elf information >
 Output		:struct elf_info *elf_info
 		 < read elf information >
 Return		:int
 		 < result >
 Description	:read elf header
==================================================================================
*/
LOCAL int read_elf32(int fd, struct elf_info *elf_info)
{
	ssize_t len;
	int err = 0;
	
	len = read_elf32_header(fd, &elf_info->hdr);
	
	//show_elf32_header(&elf_info->hdr);
	
	if (len < 0) {
		vd_printf("1:error:%s[%d]\n", __func__, len);
		err = (int)len;
		return(err);
	}
	
	if (len != sizeof(Elf32_Ehdr)) {
		vd_printf("2:error:%s[%d]\n", __func__, -len);
		err = -ENOEXEC;
		return(err);
	}
	
	err = check_elf32_header(&elf_info->hdr);
	
	if (err) {
		vd_printf("3:error:%s[%d]\n", __func__, -err);
		return(err);
	}
	
	err = read_elf32_phdr(fd, elf_info);
	
	if (UNLIKELY(err < 0)) {
		vd_printf("4:error:%s[%d]\n", __func__, -err);
		return(err);
	}
	
	if (has_pt_interp(elf_info)) {
		return(ELF_HAS_INTERP);
	}
	
	err = read_elf32_shdr(fd, elf_info);
	
	if (UNLIKELY(err < 0)) {
		vd_printf("5:error:%s[%d]\n", __func__, -err);
		return(err);
	}
	
	return(0);
}


/*
==================================================================================
 Funtion	:read_interp_name32
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:void
 Return		:char*
 		 < interp name >
 Description	:read intepreter name from a segment
==================================================================================
*/
LOCAL char* read_interp_name32(int fd, struct elf_info *elf_info)
{
	Elf32_Phdr *phdr = &elf_info->phdr[elf_info->interp_index];
	ssize_t len;
	size_t size;
	char *interp_name;
	
	size = phdr->p_memsz;
	
	interp_name = (char*)kmalloc(size, 0);
	
	if (UNLIKELY(!interp_name)) {
		vd_printf("1:%s\n", __func__);
		return(NULL);
	}
	
	len = kpread(fd, interp_name, size, phdr->p_offset);
	
	if (UNLIKELY(len != size)) {
		vd_printf("2:%s\n", __func__);
		vd_printf("len:%d size:%d p_memsz:%d p_filesz:%d\n", len, size, phdr->p_memsz, phdr->p_filesz);
		kfree(interp_name);
		return(NULL);
	}
	
	return(interp_name);
}


/*
==================================================================================
 Funtion	:load_elf32
 Input		:int fd
 		 < open file descriptor for a new program/interpreter >
 		 struct elf_info *elf_info
 		 < elf information >
 		 struct vdso_load_info *vdso_info
 		 < vdso load inforamtion >
 Output		:struct vdso_load_info *vdso_info
 		 < vdso load inforamtion >
 Return		:int
 		 < result >
 Description	:load elf
==================================================================================
*/
LOCAL int
load_elf32(int fd, struct elf_info *elf_info, struct vdso_load_info *vdso_info)
{
	Elf32_Ehdr *hdr = &elf_info->hdr;
	Elf32_Phdr *phdr = elf_info->phdr;
	//Elf32_Shdr *shdr = elf_info->shdr;
	struct process *proc = get_current();
	struct memory_space *mspace = proc->mspace;
	int i;
	int ret = 0;
	int bss_flags = 0;
	int text_flags = 0;
	void *addr;
	loff_t data_offset = 0;
	ssize_t len;
	
	/* -------------------------------------------------------------------- */
	/* intepret program headers						*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < hdr->e_phnum;i++) {
		void *vaddr;
		size_t size;
		int flags;
		int mmap_flags;
		
		if (phdr[i].p_type != PT_LOAD) {
			continue;
		}
		
		/* ------------------------------------------------------------ */
		/* PT_LOAD							*/
		/* ------------------------------------------------------------ */
		if (vdso_info) {
			vaddr = NULL;
		} else {
			vaddr = (void*)(PAGE_ALIGN(phdr[i].p_vaddr));
		}
		
		flags = phdr[i].p_flags;
		
		if (vdso_info) {
			size = RoundPage(phdr[i].p_memsz);
		} else {
			size = RoundPage((unsigned long)phdr[i].p_vaddr +
					phdr[i].p_memsz - (unsigned long)vaddr);
		}
		
		if ((flags & (PF_R | PF_W)) == (PF_R | PF_W)) {
			if (UNLIKELY(bss_flags)) {
				panic("there are double segments[1] %s\n", __func__);
			}
			/* ---------------------------------------------------- */
			/* .bss, .data and etc. section				*/
			/* ---------------------------------------------------- */
			//printf("mmap1:0x%08X 0x%08X\n", vaddr, size);
			
			mmap_flags = MAP_PRIVATE |MAP_ANONYMOUS | MAP_FIXED;
			
			/* ---------------------------------------------------- */
			/* clear MAP_FIXED for a relocatable program		*/
			/* ---------------------------------------------------- */
			if (vdso_info) {
				mmap_flags &= ~MAP_FIXED;
			}
			
			addr = mmap(vaddr, size, PROT_READ | PROT_WRITE,
					mmap_flags, -1, 0);
			
			if (UNLIKELY((long)addr < 0)) {
				printf("failed at %s[1] 0x%08X\n", __func__, addr);
				ret = -ENOMEM;
				goto map_failed;
			}
			
			//show_vm_list(get_current());
			
			data_offset = phdr[i].p_offset;
			
			/* ---------------------------------------------------- */
			/* .data section					*/
			/* ---------------------------------------------------- */
			if (vdso_info) {
				mspace->start_data = (unsigned long)addr +
							phdr[i].p_offset;;
			} else {
				mspace->start_data =
						(unsigned long)phdr[i].p_vaddr;
			}
			mspace->end_data = mspace->start_data + phdr[i].p_filesz;
			/* ---------------------------------------------------- */
			/* .bss section						*/
			/* ---------------------------------------------------- */
			mspace->start_bss = mspace->end_data;
			mspace->end_bss = mspace->start_bss +
					(phdr[i].p_memsz - phdr[i].p_filesz);
			/* ---------------------------------------------------- */
			/* set break						*/
			/* ---------------------------------------------------- */
			mspace->start_brk =
				(unsigned long)PageAlignU((void*)mspace->end_bss);
			mspace->end_brk = mspace->start_brk;
			
			bss_flags = 1;
		} else if ((flags & (PF_R | PF_X)) ==  (PF_R | PF_X)) {
			if (UNLIKELY(text_flags)) {
				panic("there are double segments[2] %s\n", __func__);
			}
			/* ---------------------------------------------------- */
			/* .text and etc. section				*/
			/* ---------------------------------------------------- */
			//size = RoundPage(phdr[i].p_filesz);
			
			//printf("mmap2:0x%08X 0x%08X\n", vaddr, size);
			
			mmap_flags = MAP_PRIVATE | MAP_FIXED;
			
			/* ---------------------------------------------------- */
			/* clear MAP_FIXED for a relocatable program		*/
			/* ---------------------------------------------------- */
			if (vdso_info) {
				mmap_flags &= ~MAP_FIXED;
			}
			
			addr = mmap(vaddr, size, PROT_READ | PROT_EXEC,
					mmap_flags, fd,
					phdr[i].p_offset);
			
			//show_vm_list(get_current());
			
			if (UNLIKELY((long)addr < 0)) {
				printf("failed at %s[3]\n", __func__);
				ret = -ENOMEM;
				goto map_failed;
			}
			
			/* ---------------------------------------------------- */
			/* .text section					*/
			/* ---------------------------------------------------- */
			if (vdso_info) {
				mspace->start_code = (unsigned long)addr;
			} else {
				mspace->start_code =
						(unsigned long)phdr[i].p_vaddr;
			}
			mspace->end_code = mspace->start_code + phdr[i].p_memsz;
			
			text_flags = 1;
			
			/* ---------------------------------------------------- */
			/* vdso load information				*/
			/* ---------------------------------------------------- */
			if (vdso_info) {
				vdso_info->vdso_base = (unsigned long)addr;
				vdso_info->vdso_entry = vdso_info->vdso_base +
					(unsigned long)elf_info->hdr.e_entry;
			}
		}
	}
	
	/* -------------------------------------------------------------------- */
	/* cannot find .bss and .text segment					*/
	/* -------------------------------------------------------------------- */
	if (UNLIKELY(!bss_flags || !text_flags)) {
		printf("failed at %s[4]\n", __func__);
		return(-ELIBBAD);
	}
	
	if (UNLIKELY(mspace->end_bss < mspace->end_code)) {
		printf("mspace->end_bss:0x%08X mspace->end_code:0x%08X\n", mspace->end_bss, mspace->end_code);
		//panic("unexpected executable file!!! [%s]\n", __func__);
	}
	
	/* -------------------------------------------------------------------- */
	/* copy .data etc. segments						*/
	/* -------------------------------------------------------------------- */
	len = kpread(fd, (char*)mspace->start_data,
			mspace->end_data - mspace->start_data, data_offset);
	
	if (UNLIKELY(len < 0)) {
		printf("failed at %s[5]\n", __func__);
		ret = (int)len;
		goto map_failed;
	}
	
	//memset((void*)mspace->start_bss, 0x00, mspace->end_bss - mspace->end_bss);
	
	return(0);
	
map_failed:
	printf("map_failed at %s\n", __func__);
	free_vm(proc);
	return(ret);
}


/*
==================================================================================
 Funtion	:relocate_elf32
 Input		:unsigned long load_addr
 		 < virtual address at which a program is actually loaded>
 		 unsigned long elf_addr
 		 < virtual address at which a elf format describes >
 Output		:void
 Return		:int
 		 < result >
 Description	:relocate values after load elf
==================================================================================
*/
LOCAL int relocate_elf32(unsigned long load_addr, unsigned long elf_addr)
{
	Elf32_Ehdr *hdr = (Elf32_Ehdr*)load_addr; // a loaded elf header
	Elf32_Phdr *dyn_phdr;
	Elf32_Dyn *dyn_entry;
	struct dynamic_info dynamic_info;
	int err;
	
	if (hdr->e_type != ET_EXEC) {
		/* do nothing							*/
		return(0);
	}
	
	/* -------------------------------------------------------------------- */
	/* get a program header of a dyanamic segment				*/
	/* -------------------------------------------------------------------- */
	dyn_phdr = get_dynamic_segment(hdr);
	
	if (!dyn_phdr) {
		/* there is no dynamic segment. do nothing			*/
		return(0);
	}
	
	if (UNLIKELY(!dyn_phdr->p_paddr)) {
		return(0);
	}
	
	/* -------------------------------------------------------------------- */
	/* get first contents of a dynamic section				*/
	/* -------------------------------------------------------------------- */
	dyn_entry = (Elf32_Dyn*)(load_addr + dyn_phdr->p_offset);
	
	/* -------------------------------------------------------------------- */
	/* read a dynamic section						*/
	/* -------------------------------------------------------------------- */
	err = read_dynamic_section((long)(load_addr - elf_addr),
					dyn_phdr, dyn_entry, &dynamic_info);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	//err = relocate_symtab((long)(load_addr - elf_addr), &dynamic_info);
}

/*
==================================================================================
 Funtion	:read_elf32_header
 Input		:int fd
 		 < open executable file descriptor >
 		 Elf32_Ehdr *buf
 		 < buffer to output a header >
 Output		:Elf32_Ehdr *buf
 		 < buffer to output a header >
 Return		:ssize_t
 		 < actual length >
 Description	:read elf32 header
==================================================================================
*/
LOCAL ssize_t read_elf32_header(int fd, Elf32_Ehdr *buf)
{
	ssize_t len;
	
	len = kpread(fd, (char*)buf, sizeof(Elf32_Ehdr), 0);
	
	return(len);
}




/*
==================================================================================
 Funtion	:check_elf32_header
 Input		:const Elf32_Ehdr *header
 		 < buffer on which elf header resides >
 Output		:void
 Return		:int
 		 < result >
 Description	:check elf header
==================================================================================
*/
LOCAL int check_elf32_header(const Elf32_Ehdr *header)
{
	if ((header->e_ident[EI_MAG0] != ELFMAG0) ||
		(header->e_ident[EI_MAG1] != ELFMAG1) ||
		(header->e_ident[EI_MAG2] != ELFMAG2) ||
		(header->e_ident[EI_MAG3] != ELFMAG3)) {
		return(-ENOEXEC);
	}
	
	if (header->e_ident[EI_CLASS] != ELFCLASS32) {
		return(-ENOEXEC);
	}
	
	if (header->e_ident[EI_DATA] != ELFDATA2LSB) {
		return(-ENOEXEC);
	}
	
	if (header->e_ident[EI_VERSION] != EV_CURRENT) {
		return(-ENOEXEC);
	}
	
	if ((header->e_type != ET_EXEC) && (header->e_type != ET_DYN)) {
		return(-ENOEXEC);
	}
	
	if ((header->e_machine != EM_386) && (header->e_machine != EM_486)) {
		return(-ENOEXEC);
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:read_elf32_phdr
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:struct elf_info *elf_info
 		 < elf information >
 Return		:int
 		 < result >
 Description	:read elf32 program headers
==================================================================================
*/
LOCAL int read_elf32_phdr(int fd, struct elf_info *elf_info)
{
	Elf32_Ehdr *header = &elf_info->hdr;
	ssize_t len;
	size_t size;
	
	size = header->e_phentsize * header->e_phnum;
	
	if (!size) {
		return(0);
	}
	
	elf_info->phdr = (Elf32_Phdr*)kmalloc(size, 0);
	
	if (UNLIKELY(!elf_info->phdr)) {
		return(-ENOMEM);
	}
	
	len = kpread(fd, (char*)elf_info->phdr, size, header->e_phoff);
	
	return((int)len);
}



/*
==================================================================================
 Funtion	:read_elf32_shdr
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:struct elf_info *elf_info
 		 < elf information >
 Return		:int
 		 < result >
 Description	:read elf32 section headers
==================================================================================
*/
LOCAL int read_elf32_shdr(int fd, struct elf_info *elf_info)
{
	Elf32_Ehdr *header = &elf_info->hdr;
	ssize_t len;
	size_t size;
	
	size = header->e_shentsize * header->e_shnum;
	
	if (!size) {
		return(0);
	}
	
	elf_info->shdr = (Elf32_Shdr*)kmalloc(size, 0);
	
	if (UNLIKELY(!elf_info->shdr)) {
		return(-ENOMEM);
	}
	
	len = kpread(fd, (char*)elf_info->shdr, size, header->e_shoff);
	
	//printf("%s\n", __func__);
	
	return((int)len);
}

/*
==================================================================================
 Funtion	:has_pt_interp
 Input		:struct elf_info *elf_info
 		 < elf information >
 Output		:void
 Return		:int
 		 < result or phdr index of interpreter >
 Description	:elf file has a PT_INTERP segment
==================================================================================
*/
LOCAL int has_pt_interp(struct elf_info *elf_info)
{
	int num = elf_info->hdr.e_phnum;
	int count;
	
	for (count = 0;count < num;count++) {
		if (UNLIKELY(elf_info->phdr[count].p_type == PT_INTERP)) {
			elf_info->interp_index = count;
			return(1);
		}
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:get_first_phdr
 Input		:Elf32_Ehdr *hdr
 		 < elf header >
 Output		:void
 Return		:Elf32_Phdr*
 		 < first program header address >
 Description	:get first program header address
==================================================================================
*/
LOCAL INLINE Elf32_Phdr* get_first_phdr(Elf32_Ehdr *hdr)
{
	Elf32_Phdr *first_phdr;
	
	first_phdr = (Elf32_Phdr*)(hdr + hdr->e_phoff);
	
	return(first_phdr);
}

/*
==================================================================================
 Funtion	:get_first_shdr
 Input		:Elf32_Ehdr *hdr
 		 < elf header >
 Output		:void
 Return		:Elf32_Shdr*
 		 < first section header address >
 Description	:get first section header address
==================================================================================
*/
LOCAL INLINE Elf32_Shdr* get_first_shdr(Elf32_Ehdr *hdr)
{
	Elf32_Shdr *first_shdr;
	
	first_shdr = (Elf32_Shdr*)(hdr + hdr->e_shoff);
	
	return(first_shdr);
}

/*
==================================================================================
 Funtion	:get_dynamic_segment
 Input		:Elf32_Ehdr *hdr
 		 < elf header >
 Output		:void
 Return		:Elf32_Phdr*
 		 < program header of dynamic segment >
 Description	:get a program header of dynamic segment
==================================================================================
*/
LOCAL Elf32_Phdr* get_dynamic_segment(Elf32_Ehdr *hdr)
{
	int num = hdr->e_phnum;
	int count;
	Elf32_Phdr *phdr = get_first_phdr(hdr);
	
	for (count = 0;count < num;count++) {
		if (UNLIKELY(phdr[count].p_type == PT_DYNAMIC)) {
			return(phdr);
		}
	}
	
	return(NULL);
}

/*
==================================================================================
 Funtion	:read_dynamic_section
 Input		:long offset
 		 < offset from loaded program address to elf header address >
 		 Elf32_Phdr *dyn_phdr
 		 < a program header of a dynamic segment >
 		 Elf32_Dyn *dyn_entry
 		 < first entry of dynamic section >
 		 struct dynamic_info *dynamic_info
 		 < output >
 Output		:struct dynamic_info *dynamic_info
 		 < read dynamic information result >
 Return		:int
 		 < resutl >
 Description	:read and intepret dynamic section
==================================================================================
*/
LOCAL int
read_dynamic_section(long offset, Elf32_Phdr *dyn_phdr,
			Elf32_Dyn *dyn_entry, struct dynamic_info *dynamic_info)
{
	int i;
	
	for (i = 0;i < (dyn_phdr->p_memsz / sizeof(Elf32_Dyn));i++) {
		unsigned long v_addr
			= (unsigned long)((long)dyn_entry[i].d_un.d_ptr + offset);
		unsigned long value = (unsigned long)dyn_entry[i].d_un.d_val;
		
		switch (dyn_entry[i].d_tag) {
		case	DT_NEEDED:
			dynamic_info->needed = (char*)v_addr;
			break;
		case	DT_HASH:
			dynamic_info->hash = (unsigned long*)v_addr;
			break;
		case	DT_STRTAB:
			dynamic_info->symtab.entry = (Elf32_Sym*)v_addr;
			break;
		case	DT_STRSZ:
			dynamic_info->symtab.syment = value;
			break;
		case	DT_RELA:
			dynamic_info->relatab.entry = (Elf32_Rela*)v_addr;
			break;
		case	DT_RELASZ:
			dynamic_info->relatab.relasz = value;
			break;
		case	DT_RELAENT:
			dynamic_info->relatab.relaent = value;
			break;
		case	DT_INIT:
			dynamic_info->func.init = v_addr;
			break;
		case	DT_FINI:
			dynamic_info->func.fini = v_addr;
			break;
		case	DT_SONAME:
			dynamic_info->soname = (char*)v_addr;
			break;
		case	DT_RPATH:
			dynamic_info->rpath = (char*)v_addr;
			break;
		case	DT_TEXTREL:
			dynamic_info->textrel = value;
			break;
		case	DT_JMPREL:
			dynamic_info->jmp.jmprel = v_addr;
			break;
		case	DT_PLTRELSZ:
			dynamic_info->jmp.pltrelsz = value;
			break;
		case	DT_PLTREL:
			dynamic_info->jmp.pltrel = value;
			break;
		default:
			break;
		}
	}
	
	return(0);
}


/*
==================================================================================
 Funtion	:show_elf32_header
 Input		:Elf32_Ehdr *ehdr
 		 < elf header >
 Output		:void
 Return		:void
 Description	:show elf header information
==================================================================================
*/
LOCAL void show_elf32_header(Elf32_Ehdr *ehdr)
{
	vd_printf("e_ident:%s\n", ehdr->e_ident);
	vd_printf("e_type:%d\n", ehdr->e_type);
	vd_printf("e_machine:%d\n", ehdr->e_machine);
	vd_printf("e_version:%d\n", ehdr->e_version);
	vd_printf("e_entry:0x%08X\n", ehdr->e_entry);
	vd_printf("e_phoff:0x%08X\n", ehdr->e_phoff);
	vd_printf("e_shoff:0x%08X\n", ehdr->e_shoff);
	vd_printf("e_flags:0x%08X\n", ehdr->e_flags);
	vd_printf("e_ehsize:%d\n", ehdr->e_ehsize);
	vd_printf("e_phentsize:%d\n", ehdr->e_phentsize);
	vd_printf("e_phnum:%d\n", ehdr->e_phnum);
	vd_printf("e_shentsize:%d\n", ehdr->e_shentsize);
	vd_printf("e_shnum:%d\n", ehdr->e_shnum);
	vd_printf("e_shstrndx:%d\n", ehdr->e_shstrndx);
}


/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
