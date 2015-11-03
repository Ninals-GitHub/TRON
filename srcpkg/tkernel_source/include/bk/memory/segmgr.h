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
 *    Modified by Nina Petipa at 2015/07/28
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
 *	segmgr.h (T2EX)
 *	T2EX: segment manager
 */

#ifndef _T2EX_SEGMGR_
#define _T2EX_SEGMGR_

/*
 * Segment manager lock
 */
IMPORT	FastLock	SegLock;
#define	LockSEG()	Lock(&SegLock)
#define	UnlockSEG()	Unlock(&SegLock)

IMPORT ER InitLogicalSpace( void );
IMPORT ER _MakeSpace( void *laddr, INT npage, INT lsid, UINT set_pte );
IMPORT ER __MakeSpace( void *laddr, INT npage, INT lsid, UINT set_pte );
IMPORT ER _UnmakeSpace( void *laddr, INT npage, INT lsid );
IMPORT ER __UnmakeSpace( void *laddr, INT npage, INT lsid );
IMPORT ER _ChangeSpace( void *laddr, INT npage, INT lsid, INT chg_pte );
IMPORT ER __ChangeSpace( void *laddr, INT npage, INT lsid, unsigned long chg_pte );
IMPORT INT _CnvPhysicalAddr( CONST void *laddr, INT len, void **paddr );
IMPORT ER _ChkSpace( CONST void *laddr, INT len, UINT mode, UINT env );
IMPORT INT _ChkSpaceLen( CONST void *laddr, INT len, UINT mode, UINT env, INT lsid );
IMPORT INT _ChkSpaceTstr( CONST TC *str, INT max, UINT mode, UINT env );
IMPORT INT _ChkSpaceBstr( CONST UB *str, INT max, UINT mode, UINT env );
IMPORT INT _SetMemoryAccess( CONST void *addr, INT len, UINT mode );

#endif /* _T2EX_SEGMGR_ */
