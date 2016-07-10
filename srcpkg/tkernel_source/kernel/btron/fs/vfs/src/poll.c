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

#include <tk/typedef.h>

#include <bk/fs/vfs.h>

#include <bk/uapi/poll.h>
#include <bk/uapi/signal.h>
#include <bk/uapi/sys/time.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int xpoll(struct pollfd *fds, nfds_t nfds,
			const struct timespec *tmo_p, const sigset_t *sigmask);

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
 Funtion	:poll
 Input		:struct pollfd *fds
 		 < poll fds >
 		 nfds_t nfds
 		 < number of poll fds >
 		 int timeout
 		 < timeout in milliseconds >
 Output		:struct pollfd *fds
 		 < poll fds >
 Return		:int
 		 < result >
 Description	:wait for some event on a file descriptor
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	struct timespec tmo;
	int err;
	
	if (UNLIKELY(!fds)) {
		return(-EFAULT);
	}
	
	err = vm_check_accessRW((void*)fds, sizeof(struct pollfd) * nfds);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	if (!timeout) {
		return(0);
	}
	
#if 0
	{
		int i;
		printf("poll:fds=0x%08X, nfds=%d, timeout=%d\n", fds, nfds, timeout);
		for(i = 0;i < nfds;i++) {
			printf("[i]fd:%d events:%d revents:%d\n", i, fds[i].fd, fds[i].events, fds[i].revents);
		}
	}
#endif
	if (0 < timeout) {
		long second = timeout / 1000;
		tmo.tv_sec = second;
		tmo.tv_nsec = (timeout - second * 1000) * 1000;
		
		err = xpoll(fds, nfds, (const struct timespec*)&tmo, NULL);
	} else {
		err = xpoll(fds, nfds, NULL, NULL);
	}
	
	return(err);
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
 Funtion	:xpoll
 Input		:struct pollfd *fds
 		 < poll fds >
 		 nfds_t nfds
 		 < number of poll fds >
 		 const struct timespec *tmo_p
 		 < timeout >
 		 const sigset_t *sigmask
 		 < signal mask >
 Output		:struct pollfd *fds
 		 < poll fds >
 Return		:int
 		 < result >
 Description	:common process for wait for some event on a file descriptor
==================================================================================
*/
LOCAL int xpoll(struct pollfd *fds, nfds_t nfds,
			const struct timespec *tmo_p, const sigset_t *sigmask)
{
	fds[0].revents = 1;
	return(1);
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
