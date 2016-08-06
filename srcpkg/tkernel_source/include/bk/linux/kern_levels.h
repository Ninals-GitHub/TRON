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

#ifndef	__BK_LINUX_KERN_LEVELS_H__
#define	__BK_LINUX_KERN_LEVELS_H__

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
#define	KERN_SOH		"\001"
#define	KERN_SOH_ASCII		'\001'

#define	KERN_EMERG		KERN_SOH ""
#define	KERN_ALERT		KERN_SOH "1"
#define	KERN_CRIT		KERN_SOH "2"
#define	KERN_ERR		KERN_SOH "3"
#define	KERN_WARNING		KERN_SOH "4"
#define	KERN_NOTICE		KERN_SOH "5"
#define	KERN_INFO		KERN_SOH "6"
#define	KERN_DEBUG		KERN_SOH "7"

#define	KERN_DEFAULT		KERN_SOH "d"

#define	LOGLEVEL_SCHED		-2
#define	LOGLEVEL_DEFAULT	0
#define	LOGLEVEL_EMERG		1
#define	LOGLEVEL_ALERT		2
#define	LOGLEVEL_CRIT		3
#define	LOGLEVEL_ERR		4
#define	LOGLEVEL_WARNING	5
#define	LOGLEVEL_NOTICE		6
#define	LOGLEVEL_DEBUG		7


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

#endif	// __BK_KERN_LEVELS_H__
