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

#ifndef	__BK_UAPI_POLL_H__
#define	__BK_UAPI_POLL_H__

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
struct pollfd {
	int	fd;		/* the following descriptor being polled	*/
	short	events;		/* the input event flags			*/
	short	revents;	/* the output event flags			*/
};

#define	POLLIN		0x0001
#define	POLLPRI		0x0002
#define	POLLOUT		0x0004
#define	POLLERR		0x0008
#define	POLLHUP		0x0010
#define	POLLNVAL	0x0020

#define	POLLRDNORM	0x0040
#define	POLLDBAND	0x0080
#define	POLLWRNORM	0x0100
#define	POLLWBAND	0x0200
#define	POLLMSG		0x0400
#define	POLLREMOVE	0x1000
#define	POLLRDHUP	0x2000
#define	POLLFREE	0x4000
#define	POLL_BUSY_LOOP	0x8000

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
SYSCALL int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif	// __BK_UAPI_POLL_H__
