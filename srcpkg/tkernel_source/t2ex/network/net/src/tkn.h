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
 *	@(#)tkn.h
 *
 */

#ifndef _TKN_H_
#define _TKN_H_

#include <basic.h>
#include <sys/syscall.h>

#include "netmain/tkn_init.h"

/*
 * Stack size definitions
 */
#define TKN_DRVTSK_STKSZ	(1536)
#define TKN_NETDMN_STKSZ	(5120)
#define	TKN_SPLTRAMPO_STKSZ	(2560)

/*
 * For mutual exclusion.
 */
IMPORT void LockTKN( void );
IMPORT void UnlockTKN( void );
IMPORT void LockSPL( void );
IMPORT void UnlockSPL( void );

/*
 * Invoke a system call.
 */
IMPORT int tkn_syscall( int sys_no, void *v, int *retval );

/*
 * BPF (Berkeley Packet Filter)
 */
IMPORT int tkn_bpf_open( const char *path, int oflag, struct file * );

/*
 * Tunneling
 */
IMPORT int tkn_tun_open( const char *path, int oflag, struct file * );

IMPORT ER init_netdmn( void );
IMPORT ER finish_netdmn( void );

#endif /* _TKN_H_ */
