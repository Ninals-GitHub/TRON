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

#ifndef	__BK_UAPI_FCNTL_H__
#define	__BK_UAPI_FCNTL_H__

#include <basic.h>
#include <stdint.h>
#include <tk/typedef.h>
#include <sys/types.h>

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
----------------------------------------------------------------------------------
	file open flags
----------------------------------------------------------------------------------
*/
#define	O_RDONLY	000000000
#define	O_WRONLY	000000001
#define	O_RDWR		000000002

#define	O_ACCMODE	000000003

#define	O_CREAT		000000100
#define	O_EXCL		000000200
#define	O_NOCTTY	000000400
#define	O_TRUNC		000001000
#define	O_APPEND	000002000
#define	O_NONBLOCK	000004000
#define	O_DSYNC		000010000
#define	FASYNC		000020000
#define	O_DIRECT	000040000
#define	O_LARGEFILE	000100000
#define	O_DIRECTORY	000200000
#define	O_NOFOLLOW	000400000
#define	O_NOATIME	001000000
#define	O_CLOEXEC	002000000

/*
----------------------------------------------------------------------------------
	fcntl command
----------------------------------------------------------------------------------
*/
#define	F_DUPFD		0
#define	F_GETFD		1	/* get close_on_exec				*/
#define	F_SETFD		2	/* set/clear close_on_exec			*/
#define	F_GETFL		3	/* get file->f_flags				*/
#define	F_SETFL		4	/* set file->f_flags				*/
#define	F_GETLK		5
#define	F_SETLK		6
#define	F_SETLKW	7
#define	F_SETOWN	8	/* for sockets					*/
#define	F_GETOWN	9	/* for sockets					*/
#define	F_SETSIG	10	/* for sockets					*/
#define	F_GETSIG	11	/* for sockets					*/

#define	F_GETLK64	12
#define	F_SETLK64	13
#define	F_SETLKW64	14

#define	F_SETOWN_EX	15
#define	F_GETOWN_EX	16

#define	F_GETOWNER_UIDS	17

/*
----------------------------------------------------------------------------------
	special file descriptor
----------------------------------------------------------------------------------
*/
#define	AT_FDCWD 		(-100)

/*
----------------------------------------------------------------------------------
	lookup flags
----------------------------------------------------------------------------------
*/
#define	AT_SYMLINK_NOFOLLOW	0x00000100	/* do not follow symbolic links	*/
#define	AT_REMOVEDIR		0x00000200	/* remove directory instead of
						   unlinking file		*/
#define	AT_SYMLINK_FOLLOW	0x00000400	/* follow symbolic links	*/
#define	AT_NO_AUTOMOUNT		0x00000800	/* suppress terminal automount
						   traversal			*/
#define	AT_EMPTY_PATH		0x00001000	/* allow empty relative pathname*/

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
 Funtion	:fcntl64
 Input		:int fd
 		 < opne file descriptor >
 		 int cmd
 		 < command >
 		 unsigned long arg
 		 < argument >
 Output		:void
 Return		:int
 		 < result >
 Description	:manipulate file descriptor
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fcntl64(int fd, int cmd, unsigned long arg);

#endif	// __BK_UAPI_FCNTL_H__
