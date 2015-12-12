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

#ifndef	__BK_UAPI_RESOURCE_H__
#define	__BK_UAPI_RESOURCE_H__


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
#define	RLIMIT_CPU		0	/* cpu time in sec			*/
#define	RLIMIT_FSIZE		1	/* maximum file size			*/
#define	RLIMIT_DATA		2	/* maximum data size			*/
#define	RLIMIT_STACK		3	/* maximum stack size			*/
#define	RLIMIT_CORE		4	/* maximum core file size		*/
#define	RLIMIT_RSS		5	/* maximum resident set size		*/
#define	RLIMIT_NPROC		6	/* maximum number of processes		*/
#define	RLIMIT_NOFILE		7	/* maximum number of open files		*/
#define	RLIMIT_MEMLOCK		8	/* maximum number of locked-in-memory	*/
					/* address spaces			*/
#define	RLIMIT_AS		9	/* address space limit			*/
#define	RLIMIT_LOCKS		10	/* maximum file locks held		*/
#define	RLIMIT_SIGPENDING	11	/* maximum number of pending signals	*/
#define	RLIMIT_MSGQUEUE		12	/* maximum bytes in POSIX mqueues	*/
#define	RLIMIT_NICE		13	/* max nice prio allowed to raise to	*/
					/* 0-39 for nice level 19 .. -20	*/
#define	RLIMIT_RTPRIO		14	/* maximum realtime priority		*/
#define	RLIMIT_RTTIME		15	/* timeout for RT tasks in us		*/

#define	RLIMIT_NLIMITS		16	/* number of limits			*/

struct rlimit {
	unsigned long	rlim_cur;	/* soft limit				*/
	unsigned long	rlim_max;	/* hard limit				*/
};

#define	RLIM_INFINITY		(~0UL)

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

#endif	// __BK_UAPI_RESOURCE_H__
