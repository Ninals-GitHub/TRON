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
 *    Modified by Nina Petipa at 2015/07/28
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
 *	memdef.h (T2EX)
 *	T2EX: memory map definitions (em1d)
 *
 *	Note: This header file is also included from assembly codes.
 */

#ifndef _T2EX_MEMDEF_
#define _T2EX_MEMDEF_

#ifndef _in_asm_source_
#include <sys/rominfo.h>
#include <sys/sysinfo.h>
#endif

/*
 * Convert logical and physical addresses
 */
#define	toLogicalAddress(paddr)		(VP)((unsigned long)(paddr) + KERNEL_BASE_ADDR)
#define	toPhysicalAddress(laddr)	(VP)((unsigned long)(laddr) - KERNEL_BASE_ADDR)

/*
 * RAM area boundaries
 */
#define	REALMEMORY_TOP	0x00000000
//#define	REALMEMORY_END	0xFFFFFFFF
#ifndef _in_asm_source_
#define	REALMEMORY_END	(getBootInfo()->lowmem_limit)
#else
#endif
/*
 * ROM area boundaries
 */
#define	ROM_TOP		0x00000000
#define	ROM_END		0x00000000

/*
 * Returns true if addr is in RAM area
 */
#define	isRAM(addr)		(  (unsigned long)(addr) >= REALMEMORY_TOP \
				&& (unsigned long)(addr) <  REALMEMORY_END )

/*
 * Returns true if addr is in ROM area
 */
/*#define	isROM(addr)		(  (UW)(addr) >= ROM_TOP \
				&& (UW)(addr) <  ROM_END )*/
#define	isROM(addr)	(0)

#endif /* _T2EX_MEMDEF_ */
