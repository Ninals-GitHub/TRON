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

#include <typedef.h>
#include <tk/task.h>
#include <t2ex/sys/types.h>
#include <bk/bprocess.h>
#include <bk/memory/page.h>
#include <bk/memory/mmap.h>
#include <bk/uapi/berrno.h>

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
#define	MMPA_PAGESIZE		PAGESIZE

/*
==================================================================================

	Management 

==================================================================================
*/
SYSCALL void* mmap(void *addr, size_t length, int prot,
			int flags, int fd, off_t offset);


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mmap
 Input		:void *addr
 		 < base address of virtual memory to request a mapping >
 		 size_t length
 		 < size of memory to request a mapping >
 		 int prot
 		 < page protection >
 		 int flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor >
 		 off_t offset
 		 < offset in a file >
 Output		:void
 Return		:void *addr
 		 < base address of mapped memory >
 Description	:make a new memory mapping
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL void* mmap(void *addr, size_t length, int prot,
			int flags, int fd, off_t offset)
{
	struct process *current = get_current();
	
	return(xmmap(current, addr, length, prot, flags, fd, offset));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mmap2
 Input		:void *addr
 		 < base address of virtual memory to request a mapping >
 		 size_t length
 		 < size of memory to request a mapping >
 		 int prot
 		 < page protection >
 		 int flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor >
 		 off_t offset
 		 < offset in a file in unit of page size >
 Output		:void
 Return		:void *addr
 		 < base address of mapped memory >
 Description	:make a new memory mapping with page size offset
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL void* mmap2(void *addr, size_t length, int prot,
			int flags, int fd, off_t pgoffset)
{
	struct process *current = get_current();
	void *allocated;
#if 0
	printf("mmap2:");
#endif
	allocated = xmmap(current, addr, length, prot,
				flags, fd, pgoffset * MMPA_PAGESIZE);
	
	return(allocated);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:xmmap
 Input		:struct process *proc
 		 < process to map memory from its memory space >
 		 void *addr
 		 < base address of virtual memory to request a mapping >
 		 size_t length
 		 < size of memory to request a mapping >
 		 int prot
 		 < page protection >
 		 int flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor >
 		 off_t offset
 		 < offset in a file >
 Output		:void
 Return		:void
 Description	:actual mmap process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* xmmap(struct process *proc, void *addr, size_t length,
			int prot, int flags, int fd, off_t offset)
{
	int err = 0;
	unsigned long start;
	unsigned long end;
	void *candidate = NULL;
	
#if 0
	printf("mmap[addr=0x%08X", addr);

	printf(", length=%d", length);
	printf(", prot=0x%08X", prot);
	printf(", flags=0x%08X", flags);
	printf(", fd=%d", fd);
	printf(", offset=%d]\n", offset);
#endif
	if (UNLIKELY(addr && ((unsigned long)addr & ~PAGE_MASK))) {
		//printf("mmap failed:address alignment 0x%08X\n", ((unsigned long)addr & ~PAGE_MASK));
		return(MAP_FAILED);
	}
	
	if (UNLIKELY(offset & ~PAGE_MASK)) {
		//printf("mmap failed:offset alignment\n");
		return(MAP_FAILED);
	}
	
	if (UNLIKELY(!addr && !(flags & MAP_FIXED))) {
		/* ------------------------------------------------------------ */
		/* search candidate mmap area					*/
		/* ------------------------------------------------------------ */
		candidate = search_mmap_candidate(proc, length, prot,
							flags, fd, offset);
		
		if (candidate == MAP_FAILED) {
			//printf("cannot search mmap candidate\n");
			return(MAP_FAILED);
		}
		start = (unsigned long)candidate;
		//printf("mmap:new addr:0x%08X\n", start);
	} else {
		start = (unsigned long)addr;
	}
	
	end  = start + length;
	
	if (!candidate) {
		err = unmap_vm(proc, start, end);
	}
	
	if (UNLIKELY(err)) {
		//printf("mmpa failed:unmap existing memory\n");
		return(MAP_FAILED);
	}
	
	if (!(flags & MAP_ANONYMOUS)) {
		/* ------------------------------------------------------------ */
		/* as for now, map file to memory is not suppoerted 		*/
		/* future work:							*/
		/* ------------------------------------------------------------ */
		err = map_vm(proc, start, end, prot);
	} else {
		err = map_vm_annon(proc, start, end, prot);
	}
	
	
	if (UNLIKELY(err)) {
		//printf("mmap failed:map_vm failed\n");
		return(MAP_FAILED);
	}
	
	return((void*)start);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:brk
 Input		:unsigned long addr
 		 < new break address >
 Output		:void
 Return		:unsigned long
 		 < new break address >
 Description	:change data segment size
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL unsigned long brk(unsigned long addr)
{
	unsigned long new_brk;
	unsigned long new_size;
	struct process *proc = get_current();
	struct memory_space *mspace = proc->mspace;
	
	//printf("brk[addr=0x%08X]", addr);
	
	if (UNLIKELY(addr < mspace->start_brk)) {
		//printf("<1:new addr=0x%08X>\n", mspace->end_brk);
		return(mspace->end_brk);
	}
	
	if (addr < mspace->end_brk) {
		int err;
		/* ------------------------------------------------------------ */
		/* shrink data segment						*/
		/* ------------------------------------------------------------ */
		err = munmap((void*)addr, mspace->end_brk - addr);
		
		if (UNLIKELY(err)) {
			return(mspace->end_brk);
		}
		
		mspace->end_brk = addr;
		//printf("<2:new addr=0x%08X>\n", mspace->end_brk);
		return(mspace->end_brk);
	}
	
	new_size =
		(unsigned long)PageAlignU((const void*)(addr - mspace->end_brk));
	
	if (proc->rlimits[RLIMIT_DATA].rlim_cur <
			(new_size + (mspace->end_data - mspace->start_data))) {
		//printf("<5:new addr=0x%08X>\n", mspace->end_brk);
		return(mspace->end_brk);
	}
	
	new_brk = (unsigned long)mmap((void*)mspace->end_brk, new_size,
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE | MAP_ANONYMOUS,
					-1, 0);
	
	if (UNLIKELY((void*)new_brk == MAP_FAILED)) {
		return(mspace->end_brk);
	}
	
	mspace->end_brk = new_brk + new_size;
	
	//printf("<4:new addr=0x%08X>\n", mspace->end_brk);
	
	return(mspace->end_brk);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:munmap
 Input		:void *addr
 		 < start address of mapped memory >
 		 size_t length
 		 < length of mapped memory >
 Output		:void
 Return		:int
 		 < result >
 Description	:unmap mapped memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int munmap(void *addr, size_t length)
{
	printf("munmap[addr=0x%08X", addr);
	printf(", length=%d\n", length);
	return(unmap_vm(get_current(),
			(unsigned long)addr, (unsigned long)addr + length));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:sbrk
 Input		:long addr
 		 < increment of new break address >
 Output		:void
 Return		:unsigned long
 		 < new break address >
 Description	:change data segment size
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL unsigned long sbrk(long addr)
{
	struct process *proc = get_current();
	struct memory_space *mspace = proc->mspace;
	
	if (UNLIKELY(!addr)) {
		return(mspace->end_brk);
	}
	
	return(brk(mspace->end_brk + addr));
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
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
