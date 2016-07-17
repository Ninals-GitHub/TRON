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

#ifndef	__BK_UTIME_H__
#define	__BK_UTIME_H__

#include <bk/typedef.h>

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
struct utimbuf {
	time_t		actime;		/* access time				*/
	time_t		modtime;	/* modification time			*/
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
 Funtion	:utime
 Input		:const char *filename
 		 < file path to change its access time and modification time >
 		 const struct utimbuf *times
 		 < access time and modification time to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change file last access and modification times
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int utime(const char *filename, const struct utimbuf *times);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:utimes
 Input		:const char *filename
 		 < file path to change its access time and modification time >
 		 const struct timeval times[2]
 		 < access time and modification time to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change file last access and modification times
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int utimes(const char *filename, const struct timeval times[2]);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_current_timeval
 Input		:struct timespec *time
 		 < time value to set current time >
 Output		:struct timespec *time
 		 < time value to set current time >
 Return		:void
 Description	:get current time as timeval
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void get_current_timeval(struct timespec *time);

#endif	// __BK_UTIME_H__
