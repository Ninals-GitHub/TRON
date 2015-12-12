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

#ifndef	__BK_UAPI_SYSDEPEND_STD_X86_H__
#define	__BK_UAPI_SYSDEPEND_STD_X86_H__

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
struct user_desc {
	unsigned int	entry_number;		/* entry number of GDT		*/
	unsigned long	base_addr;		/* base address of LDT		*/
	unsigned int	limit;			/* limit of LDT			*/
	unsigned int	seg_32bit:1;		/* indicates D/B bit of GDT	*/
						/* [0:16-bit 1:32bit]		*/
	unsigned int	contents:2;		/* indicates upper 2 bits of	*/
						/* type bit of GDT		*/
	unsigned int	read_exec_only:1;	/* [0:r/w 1:read only] indicates*/
						/* 1st bit of type bit of GDT.	*/
						/* NOTE:bit is inverted against	*/
						/* actual type bit		*/
	unsigned int	limit_in_pages:1;	/* [0:unit in byte 1:unit in 4k]*/
						/* indicates G bit of GDT.	*/
	unsigned int	seg_not_present:1;	/* [0:present 1:not present]	*/
						/* indicates P bit of GDT.	*/
						/* NOTE:bit is inverted against	*/
						/* actual type bit		*/
	unsigned int	useable:1;		/* [0:not usable 1:usable]	*/
						/* indicates AVL bit of GDT	*/
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


#endif	// __BK_UAPI_SYSDEPEND_STD_X86_H__
