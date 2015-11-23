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
 *	@(#)fimp_fat.h
 *
 */

#ifndef __FIMP_FAT_H__
#define __FIMP_FAT_H__

#include <t2ex/fs.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 *  Configuration value : SYSCONF
 */
#define SYSCONF_MaxOpenF		(UB*)"FsMaxFile"
#define SYSCONF_TaskPriority		(UB*)"FiFAT_TskPri"
#define SYSCONF_TaskStackSize		(UB*)"FiFAT_StkSz"
#define SYSCONF_CacheFATMemorySize	(UB*)"FiFAT_FCacheSz"
#define SYSCONF_CacheFATRatio		(UB*)"FiFAT_FCacheNs"
#define SYSCONF_CacheRootMemorySize	(UB*)"FiFAT_RCacheSz"
#define SYSCONF_CacheRootRatio		(UB*)"FiFAT_RCacheNs"
#define SYSCONF_CacheDataMemorySize	(UB*)"FiFAT_DCacheSz"
#define SYSCONF_CacheDataRatio		(UB*)"FiFAT_DCacheNs"
#define SYSCONF_LastAccess		(UB*)"FsAccessTime"

#define DEFAULT_MaxOpenF		(64)
#define DEFAULT_TaskPriority		(100)
#define DEFAULT_TaskStackSize		(2 * 1024)
#define DEFAULT_CacheFATMemorySize	(320 * 1024)
#define DEFAULT_CacheFATRatio		(4)
#define DEFAULT_CacheRootMemorySize	(16 * 1024)
#define DEFAULT_CacheRootRatio		(4)
#define DEFAULT_CacheDataMemorySize	(1024 * 1024)
#define DEFAULT_CacheDataRatio		(64)
#define DEFAULT_LastAccess		(1)

/*
 *  FATFS FIMP entry
 */
IMPORT	const	fs_fimp_t	fimp_fatfs_entry;

#ifdef	__cplusplus
}
#endif

#endif	/* __FIMP_FAT_H__ */

