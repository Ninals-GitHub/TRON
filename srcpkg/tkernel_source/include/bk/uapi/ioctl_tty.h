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

#ifndef	__BK_UAPI_IOCTL_TTY_H__
#define	__BK_UAPI_IOCTL_TTY_H__

#include <bk/uapi/ioctl.h>

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
#define	TCGETS		_IO('T', 0x01)
#define	TCSETS		_IO('T', 0x02)
#define	TCSETSW		_IO('T', 0x03)
#define	TCSETSF		_IO('T', 0x04)
#define	TCGETA		_IO('T', 0x05)
#define	TCSETA		_IO('T', 0x06)
#define	TCSETAW		_IO('T', 0x07)
#define	TCSETAF		_IO('T', 0x08)
#define	TCSBRK		_IO('T', 0x09)
#define	TCXONC		_IO('T', 0x0A)
#define	TCFLSH		_IO('T', 0x0B)
#define	TIOCEXCL	_IO('T', 0x0C)
#define	TIOCNXCL	_IO('T', 0x0D)
#define	TIOCSCTTY	_IO('T', 0x0E)
#define	TIOCGPGRP	_IO('T', 0x0F)
#define	TIOCSPGRP	_IO('T', 0x10)
#define	TIOCOUTQ	_IO('T', 0x11)
#define	TIOCSTI		_IO('T', 0x12)
#define	TIOCGWINSZ	_IO('T', 0x13)
#define	TIOCSWINSZ	_IO('T', 0x14)
#define	TIOCMGET	_IO('T', 0x15)
#define	TIOCMBIS	_IO('T', 0x16)
#define	TIOCMBIC	_IO('T', 0x17)
#define	TIOCMSET	_IO('T', 0x18)
#define	TIOCMGSOFTCAR	_IO('T', 0x19)
#define	TIOCMSSOFTCAR	_IO('T', 0x1A)
#define	FIONREAD	_IO('T', 0x1B)
#define	TIOCINQ		_IO('T', 0x1B)
#define	TIOCLINUX	_IO('T', 0x1C)
#define	TIOCCONS	_IO('T', 0x1D)
#define	TIOCGSERIAL	_IO('T', 0x1E)
#define	TIOCSSERIAL	_IO('T', 0x1F)
#define	TIOCPKT		_IO('T', 0x20)
#define	FIONBIO		_IO('T', 0x21)
#define	TIOCNOTTY	_IO('T', 0x22)
#define	TIOCSETD	_IO('T', 0x23)
#define	TIOCGETD	_IO('T', 0x24)
#define	TCSBRKP		_IO('T', 0x25)	// for posix tcsendbrea()
#define	TIOCSBRK	_IO('T', 0x27)	// for bsd compatibility
#define	TIOCCBRK	_IO('T', 0x28)	// for bsd compatibility
#define	TIOCGSID	_IO('T', 0x29)	// return the session id of fd
#define	TCGETS2		_IOR('T', 0x2A, struct termios2)
#define	TCSETS2		_IOW('T', 0x2B, struct termios2)
#define	TCSETSW2	_IOW('T', 0x2C, struct termios2)
#define	TCSETSF2	_IOW('T', 0x2D, struct termios2)
#define	TIOCGRS485	_IO('T', 0x2E)
#define	TIOCSRS485	_IO('T', 0x2F)
#define	TIOCGPTN	_IOR('T', 0x30, unsigned int)	// get pty number
#define	TIOCSPTLCK	_IOW('T', 0x31, int)		// lock/unlock pty
#define	TIOCGDEV	_IOR('T', 0x32, unsigned int)	// get primary device node
							// of /dev/console
#define	TCGETX		_IO('T', 0x32)	// sys5 tcgetx compatibility
#define	TCSETX		_IO('T', 0x33)
#define	TCSETXF		_IO('T', 0x34)
#define	TCSETXW		_IO('T', 0x35)
#define	TIOCSIG		_IOW('T', 0x36, int)		// pty: generate signal
#define	TIOCVHANGUP	_IO('T', 0x37)
#define	TIOCGPKT	_IOR('T', 0x38, int)		// get packet mode state
#define	TIOCGPTLCK	_IOR('T', 0x39, int)		// get pty lock state
#define	TIOCGEXCL	_IOR('T', 0x40, int)		// get exclusive mode state
#define	FIONCLEX	_IO('T', 0x50)
#define	FIOCLEX		_IO('T', 0x51)
#define	FIOASYNC	_IO('T', 0x52)
#define	TIOCSERCONFIG	_IO('T', 0x53)
#define	TIOCSERGWILD	_IO('T', 0x54)
#define	TIOCSERSWILD	_IO('T', 0x55)
#define	TIOCGLCKTRMIOS	_IO('T', 0x56)
#define	TIOCSLCKTRMIOS	_IO('T', 0x57)
#define	TIOCSERGSTRUCT	_IO('T', 0x58)	// for debugging only
#define	TIOCSERGETLSR	_IO('T', 0x59)	// get line status register
#define	TIOCSERGETMULTI	_IO('T', 0x5A)	// get multiport config
#define	TIOCSERSETMULTI	_IO('T', 0x5B)	// set multiport config
#define	TIOCMIWAIT	_IO('T', 0x5C)	// wait for a change on serial input line(s)
#define	TIOCGICOUNT	_IO('T', 0x5D)	// read serial port inline interrupt counts

#define	FIOQSIZE	_IO('T', 0x60)


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

#endif	// __BK_UAPI_IOCTL_TTY_H__
