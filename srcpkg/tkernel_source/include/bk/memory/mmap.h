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

#ifndef	__BK_MMAP_H__
#define	__BK_MMAP_H__

#include <t2ex/sys/types.h>

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
#define MAP_FAILED	((void*)-1)

#define	MAP_SHARED	MAKE_BIT32(0)	/* share changes			*/
#define	MAP_PRIVATE	MAKE_BIT32(1)	/* changes are private			*/
#define	MAP_TYPE	MAKE_MASK32(0,3)/* mask for type of mapping		*/
#define	MAP_FIXED	MAKE_BIT32(4)	/* interpret addr exactly		*/
#define	MAP_ANONYMOUS	MAKE_BIT32(5)	/* do not use a file			*/

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
IMPORT void* mmap(void *addr, size_t length, int prot,
			int flags, int fd, off_t offset);

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
IMPORT void* mmap2(void *addr, size_t length, int prot,
			int flags, int fd, off_t pgoffset);

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
IMPORT int munmap(void *addr, size_t length);

#endif	// __BK_MMAP_H__
