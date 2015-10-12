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
 *	@(#)rominfo.h
 *
 */

#ifndef _ROMINFO_H_
#define _ROMINFO_H_

#include <basic.h>
#include <tk/tkernel.h>

#define SCTAG_SOMAXSOCKET	((UB*)"SoMaxSocket")
#define SCTAG_SOTASKBASEPRI	((UB*)"SoTaskBasePri")
#define SCTAG_SODRVRXBUFNUM	((UB*)"SoDrvRxBufNum")
#define SCTAG_SOTCPTXBUFSZ	((UB*)"SoTcpTxBufSz")
#define SCTAG_SOTCPRXBUFSZ	((UB*)"SoTcpRxBufSz")
#define SCTAG_SOUDPTXBUFSZ	((UB*)"SoUdpTxBufSz")
#define SCTAG_SOUDPRXBUFSZ	((UB*)"SoUdpRxBufSz")
#define SCTAG_SORAWIPTXBUFSZ	((UB*)"SoRawIPTxBufSz")
#define SCTAG_SORAWIPRXBUFSZ	((UB*)"SoRawIPRxBufSz")
#define SCTAG_SORAWTXBUFSZ	((UB*)"SoRawTxBufSz")
#define SCTAG_SORAWRXBUFSZ	((UB*)"SoRawRxBufSz")
#define SCTAG_SOTCPDOAUTOTX	((UB*)"SoTcpDoAutoTx")
#define SCTAG_SOTCPINCAUTOTX	((UB*)"SoTcpIncAutoTx")
#define SCTAG_SOTCPMAXAUTOTX	((UB*)"SoTcpMaxAutoTx")
#define SCTAG_SOTCPDOAUTORX	((UB*)"SoTcpDoAutoRx")
#define SCTAG_SOTCPINCAUTORX	((UB*)"SoTcpIncAutoRx")
#define SCTAG_SOTCPMAXAUTORX	((UB*)"SoTcpMaxAutoRx")

#define SCTAG_SOMAXMUTEX	((UB*)"SoMaxMutex")
#define SCTAG_SOMAXCONDVAR	((UB*)"SoMaxCondvar")

#endif
