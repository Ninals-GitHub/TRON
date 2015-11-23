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
 *	T2EX SVC function code
 *
 *	   (generated automatically)
 */

#include <t2ex/ssid.h>

#define FS_FS_REGIST_FN	(0x00010300 | FS_SVC)
#define FS_FS_UNREGIST_FN	(0x00020100 | FS_SVC)
#define FS_FS_ATTACH_FN	(0x00030500 | FS_SVC)
#define FS_FS_DETACH_FN	(0x00040100 | FS_SVC)
#define FS_FS_BREAK_FN	(0x00050100 | FS_SVC)
#define FS_FS_CHDIR_FN	(0x00060100 | FS_SVC)
#define FS_FS_FCHDIR_FN	(0x00070100 | FS_SVC)
#define FS_FS_GETCWD_FN	(0x00080200 | FS_SVC)
#define FS_FS_CHMOD_FN	(0x00090200 | FS_SVC)
#define FS_FS_FCHMOD_FN	(0x000a0200 | FS_SVC)
#define FS_FS_FSYNC_FN	(0x000b0100 | FS_SVC)
#define FS_FS_FDATASYNC_FN	(0x000c0100 | FS_SVC)
#define FS_FS_SYNC_FN	(0x000d0000 | FS_SVC)
#define FS_FS_MKDIR_FN	(0x000e0200 | FS_SVC)
#define FS_FS_RMDIR_FN	(0x000f0100 | FS_SVC)
#define FS_FS_GETDENTS_FN	(0x00100300 | FS_SVC)
#define FS_FS_READ_FN	(0x00110300 | FS_SVC)
#define FS_FS_WRITE_FN	(0x00120300 | FS_SVC)
#define FS_FS_CLOSE_FN	(0x00130100 | FS_SVC)
#define FS_FS_RENAME_FN	(0x00140200 | FS_SVC)
#define FS_FS_UNLINK_FN	(0x00150100 | FS_SVC)
#define FS_FS_STAT_FN	(0x00160200 | FS_SVC)
#define FS_FS_STAT_US_FN	(0x00170200 | FS_SVC)
#define FS_FS_STAT_MS_FN	(0x00180200 | FS_SVC)
#define FS_FS_STAT64_FN	(0x00190200 | FS_SVC)
#define FS_FS_STAT64_US_FN	(0x001a0200 | FS_SVC)
#define FS_FS_STAT64_MS_FN	(0x001b0200 | FS_SVC)
#define FS_FS_FSTAT_FN	(0x001c0200 | FS_SVC)
#define FS_FS_FSTAT_US_FN	(0x001d0200 | FS_SVC)
#define FS_FS_FSTAT_MS_FN	(0x001e0200 | FS_SVC)
#define FS_FS_FSTAT64_FN	(0x001f0200 | FS_SVC)
#define FS_FS_FSTAT64_US_FN	(0x00200200 | FS_SVC)
#define FS_FS_FSTAT64_MS_FN	(0x00210200 | FS_SVC)
#define FS_FS_TRUNCATE_FN	(0x00220200 | FS_SVC)
#define FS_FS_TRUNCATE64_FN	(0x00230200 | FS_SVC)
#define FS_FS_FTRUNCATE_FN	(0x00240200 | FS_SVC)
#define FS_FS_FTRUNCATE64_FN	(0x00250200 | FS_SVC)
#define FS_FS_UTIMES_FN	(0x00260200 | FS_SVC)
#define FS_FS_UTIMES_US_FN	(0x00270200 | FS_SVC)
#define FS_FS_UTIMES_MS_FN	(0x00280200 | FS_SVC)
#define FS_FS_STATVFS_FN	(0x00290200 | FS_SVC)
#define FS_FS_FSTATVFS_FN	(0x002a0200 | FS_SVC)
#define FS__FS_OPEN_FN	(0x002b0300 | FS_SVC)
#define FS__FS_LSEEK_FN	(0x002c0400 | FS_SVC)
#define FS__FS_LSEEK64_FN	(0x002d0400 | FS_SVC)
#define FS__FS_IOCTL_FN	(0x002e0300 | FS_SVC)
#define FS__FS_FCNTL_FN	(0x002f0300 | FS_SVC)

