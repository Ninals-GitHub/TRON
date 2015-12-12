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

#ifndef	__BK_AUXVEC_H__
#define	__BK_AUXVEC_H__

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
/* symbolic values for the entries in the auxiliary table put on the initial stack */
#define	AT_NULL		0	/* end of vector				*/
#define	AT_IGNORE	1	/* entry should be ignored			*/
#define	AT_EXECFD	2	/* file descriptor of program			*/
#define	AT_PHDR		3	/* program headers for program			*/
#define	AT_PHENT	4	/* size of program header entry			*/
#define	AT_PHNUM	5	/* number of program headers			*/
#define	AT_PAGESZ	6	/* system page size				*/
#define	AT_BASE		7	/* base address of interpreter			*/
#define	AT_FLAGS	8	/* flags					*/
#define	AT_ENTRY	9	/* entry point of program			*/
#define	AT_NOTELF	10	/* program is not ELF				*/
#define	AT_UID		11	/* real uid					*/
#define	AT_EUID		12	/* effective uid				*/
#define	AT_GID		13	/* real gid					*/
#define	AT_EGID		14	/* effective gid				*/
#define	AT_PLATFORM	15	/* string identifying CPU for optimizations	*/
#define	AT_HWCAP	16	/* arch dependent hints at CPU capabilities	*/
#define	AT_CLKTCK	17	/* frequency at which times() increments	*/
/* 18 - 22 are reserved								*/

/* this entry gives some information about the FPU initialization performed by
   the kernel									*/
#define	AT__FPUCW	18	/* used FPU control word			*/

/* cache block sizes								*/
#define	AT_DCACHEBSIZE	19	/* data cache block size			*/
#define	AT_ICACHEBSIZE	20	/* instruction cache block size			*/
#define	AT_UCACHEBSIZE	21	/* unified cache block size			*/

/* a special ignored value fo PPC, used by the kernel to control the
   interpretation of the aux vector. must be > 16				*/
#define	AT_IGNOREPPC	22	/* entry should be ignored			*/
#define	AT_SECURE	23	/* secure mode boolean				*/

#define	AT_BASE_PLATFORM 24	/* string identifying real platform, may differ	*/
				/* from AT_PLATFORM				*/

#define	AT_RANDOM	25	/* address of 16 random bytes			*/
#define	AT_HWCAP2	26	/* extension of AT_HWCAP			*/

#define	AT_EXECFN	31	/* filename of program				*/
/* pointer to the global system page used for system calls and other things	*/
#define	AT_SYSINFO	32
#define	AT_SYSINFO_EHDR	33

/* shapes of the caches.
   bits 0-3 contains associativity
   bits 4-7 contains log2 of line size
   mask those to get cache size							*/
#define	AT_L1I_CACHESHAPE	34
#define	AT_L1D_CACHESHAPE	35
#define	AT_L2_CACHESHAPE	36
#define	AT_L3_CACHESHAPE	37

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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_AUXVEC_H__
