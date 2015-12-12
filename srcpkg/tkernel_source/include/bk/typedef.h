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

#ifndef	__BK_TYPEDEF_H__
#define	__BK_TYPEDEF_H__

#include <typedef.h>

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
typedef W		ERR;
typedef	W		WERR;

struct ErrCode{
	W		err;	/* error code					*/
	struct {
		UH	detail;
		H	eclass;	/* error class					*/
	} c;
};

typedef int			pid_t;

typedef unsigned long long	cputime_t;
typedef unsigned long long	clock_t;

typedef unsigned long		pte_t;
typedef unsigned long		pde_t;

typedef	long			time_t;		/* Date/time			*/

typedef	unsigned int		ino_t;		/* File serial number		*/

typedef	long			off_t;		/* File size (offset)		*/

typedef	long long		off64_t;	/* File size (offset)		*/

typedef	unsigned int		mode_t;		/* File attribute		*/
typedef	unsigned int		nlink_t;	/* Number of links		*/
typedef	unsigned int		dev_t;		/* Device number		*/
typedef	unsigned int		uid_t;		/* User ID			*/
typedef	unsigned int		gid_t;		/* Group ID			*/
typedef	long			blksize_t;	/* Block size			*/
typedef	long long		blkcnt_t;	/* Number of blocks		*/

typedef	unsigned long long	fsblkcnt_t;	/* Number of blocks		*/
typedef	unsigned long long	fsfilcnt_t;	/* Number of files		*/

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

#endif	// __BK_TYPEDEF_H__
