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

#include <bk/fs/vfs.h>
#include <bk/uapi/fcntl.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int xfcntl(int fd, int cmd, unsigned long arg);

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
SYSCALL int fcntl64(int fd, int cmd, unsigned long arg)
{
	return(xfcntl(fd, cmd, arg));
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
 Funtion	:xfcntl
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
==================================================================================
*/
LOCAL int xfcntl(int fd, int cmd, unsigned long arg)
{
	struct file *filp;
	int err;
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);

	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	switch (cmd) {
	case	F_DUPFD:
		printf("fcntl:not implemented F_DUPFD\n");
		for(;;);
		break;
	case	F_GETFD:
		printf("fcntl:not implemented F_GETFD\n");
		for(;;);
		break;
	case	F_SETFD:
		printf("fcntl:not implemented F_SETFD\n");
		for(;;);
		break;
	case	F_GETFL:
		return(filp->f_flags);
	case	F_SETFL:
		filp->f_flags = (unsigned int)arg;
		break;
	case	F_GETLK:
	case	F_SETLK:
	case	F_SETLKW:
	case	F_SETOWN:
	case	F_GETOWN:
	case	F_SETSIG:
	case	F_GETSIG:
	case	F_GETLK64:
	case	F_SETLK64:
	case	F_SETLKW64:
	case	F_SETOWN_EX:
	case	F_GETOWN_EX:
	case	F_GETOWNER_UIDS:
	default:
		printf("fcntl:not implemented cmd[%d]\n", cmd);
		for(;;);
		break;
	}
	
	return(err);
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
