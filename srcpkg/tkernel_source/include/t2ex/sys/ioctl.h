/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *	@(#)ioctl.h
 *
 */

#ifndef _SYS_IOCTL_H_
#define _SYS_IOCTL_H_

#include <net/if.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* ioctl command structure:
    bits 29-31	direction
    bits 16-28	parameter length
    bits  8-15	cmd group
    bits  0-7	command
 */
#define	IOCPARM_MASK	0x1fff		/* parameter length, at most 13 bits */
#define	IOCPARM_SHIFT	16
#define	IOCGROUP_SHIFT	8

				/* no parameters */
#define	IOC_VOID	(unsigned long)0x20000000
				/* copy parameters out */
#define	IOC_OUT		(unsigned long)0x40000000
				/* copy parameters in */
#define	IOC_IN		(unsigned long)0x80000000
				/* copy parameters in and out */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
				/* mask for IN/OUT/VOID */
#define	IOC_DIRMASK	(unsigned long)0xe0000000

#define	_IOC(inout,group,num,len) \
	(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define	_IO(g,n)	_IOC(IOC_VOID,	(g), (n), 0)
#define	_IOR(g,n,t)	_IOC(IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW(g,n,t)	_IOC(IOC_IN,	(g), (n), sizeof(t))
/* this should be _IORW, but stdio got there first */
#define	_IOWR(g,n,t)	_IOC(IOC_INOUT,	(g), (n), sizeof(t))

/*
 * command values
 */
#define	FIONBIO		_IOW('f', 126, int)	/* set non-blocking I/O mode */
#define	FIONREAD	_IOR('f', 127, int)	/* get the number of bytes for reading */
#define	FIONWRITE	_IOR('f', 121, int)	/* get the number of bytes of send queue */
#define	FIONSPACE	_IOR('f', 120, int)	/* get the free space of send queue */

#define	SIOCATMARK	_IOR('s',  7, int)		/* at oob mark? */
#define	SIOCSIFADDR	_IOW('i', 12, struct ifreq)	/* set ifnet address */
#define	SIOCSIFDSTADDR	_IOW('i', 14, struct ifreq)	/* set p-p address */
#define	SIOCSIFFLAGS	_IOW('i', 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR('i', 17, struct ifreq)	/* get ifnet flags */
#define	SIOCSIFBRDADDR	_IOW('i', 19, struct ifreq)	/* set broadcast addr */
#define	SIOCSIFNETMASK	_IOW('i', 22, struct ifreq)	/* set net addr mask */
#define	SIOCDIFADDR	_IOW('i', 25, struct ifreq)	/* delete IF addr */
#define	SIOCAIFADDR	_IOW('i', 26, struct ifaliasreq)	/* add/chg IF alias */
#define	SIOCGIFADDR	_IOWR('i', 33, struct ifreq)	/* get ifnet address */
#define	SIOCGIFDSTADDR	_IOWR('i', 34, struct ifreq)	/* get p-p address */
#define	SIOCGIFBRDADDR	_IOWR('i', 35, struct ifreq)	/* get broadcast addr */
#define	SIOCGIFNETMASK	_IOWR('i', 37, struct ifreq)	/* get net addr mask */
#define SIOCINQ		FIONREAD
#define SIOCOUTQ	FIONWRITE

#define BIOCGETIF	 _IOR('B',107, struct ifreq)
#define BIOCSETIF	 _IOW('B',108, struct ifreq)

#ifdef	__cplusplus
}
#endif
#endif	/* _SYS_IOCTL_H_ */

