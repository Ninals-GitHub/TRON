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
EXPORT void* mmap(void *addr, size_t length, int prot,
			int flags, int fd, off_t offset)
{
	int err;
	struct process *current = get_current();
	unsigned long start = (unsigned long)addr + offset;
	unsigned long end = start + length;
	
	if (addr && ((unsigned long)addr & ~PAGE_MASK)) {
		printf("mmap failed:address alignment 0x%08X\n", ((unsigned long)addr & ~PAGE_MASK));
		return(MAP_FAILED);
	}
	
	if (offset & ~PAGE_MASK) {
		printf("mmap failed:offset alignment\n");
		return(MAP_FAILED);
	}
	
	err = unmap_vm(current, start, end);
	
	if (err) {
		printf("mmpa failed:unmap existing memory\n");
		return(MAP_FAILED);
	}
	
	if (!(flags & MAP_ANONYMOUS)) {
		/* ------------------------------------------------------------ */
		/* as for now, map file to memory is not suppoerted 		*/
		/* future work:							*/
		/* ------------------------------------------------------------ */
		err = map_vm(current, start, end, prot);
	} else {
		err = map_vm(current, start, end, prot);
	}
	
	if (err) {
		printf("mmap failed:map_vm failed\n");
		return(MAP_FAILED);
	}
	
	return(addr);
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
EXPORT int munmap(void *addr, size_t length)
{
	return(unmap_vm(get_current(), (unsigned long)addr, length));
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
