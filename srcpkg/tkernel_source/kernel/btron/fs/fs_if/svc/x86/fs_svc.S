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

#include <machine.h>
#include <sys/svc/svc.h>
#include "fnfs.h"

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
 Funtion	:fs_regist - _fs_fcntl
 Input		:void
 Output		:void
 Return		:void
 Description	:svc for filesystem subsystem
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
define_svc_arg3 fs_regist		FS_FS_REGIST_FN
define_svc_arg1 fs_unregist		FS_FS_UNREGIST_FN
define_svc_arg4 fs_attach		FS_FS_ATTACH_FN
define_svc_arg1 fs_detach		FS_FS_DETACH_FN
define_svc_arg1 fs_break		FS_FS_BREAK_FN
define_svc_arg1 fs_chdir		FS_FS_CHDIR_FN
define_svc_arg1 fs_fchdir		FS_FS_FCHDIR_FN
define_svc_arg2 fs_getcwd		FS_FS_GETCWD_FN
define_svc_arg2 fs_chmod		FS_FS_CHMOD_FN
define_svc_arg2 fs_fchmod		FS_FS_FCHMOD_FN
define_svc_arg1 fs_fsync		FS_FS_FSYNC_FN
define_svc_arg1 fs_fdatasync		FS_FS_FDATASYNC_FN
define_svc_arg0 fs_sync			FS_FS_SYNC_FN
define_svc_arg2 fs_mkdir		FS_FS_MKDIR_FN
define_svc_arg1 fs_rmdir		FS_FS_RMDIR_FN
define_svc_arg4 fs_getdents		FS_FS_GETDENTS_FN
define_svc_arg3 fs_read			FS_FS_READ_FN
define_svc_arg3 fs_write		FS_FS_WRITE_FN
define_svc_arg1 fs_close		FS_FS_CLOSE_FN
define_svc_arg2 fs_rename		FS_FS_RENAME_FN
define_svc_arg1 fs_unlink		FS_FS_UNLINK_FN
define_svc_arg2 fs_stat			FS_FS_STAT_FN
define_svc_arg2 fs_stat_us		FS_FS_STAT_US_FN
define_svc_arg2 fs_stat_ms		FS_FS_STAT_MS_FN
define_svc_arg2 fs_stat64		FS_FS_STAT64_FN
define_svc_arg2 fs_stat64_us		FS_FS_STAT64_US_FN
define_svc_arg2 fs_stat64_ms		FS_FS_STAT64_MS_FN
define_svc_arg2 fs_fstat		FS_FS_FSTAT_FN
define_svc_arg2 fs_fstat_us		FS_FS_FSTAT_US_FN
define_svc_arg2 fs_fstat_ms		FS_FS_FSTAT_MS_FN
define_svc_arg2 fs_fstat64		FS_FS_FSTAT64_FN
define_svc_arg2 fs_fstat64_us		FS_FS_FSTAT64_US_FN
define_svc_arg2 fs_fstat64_ms		FS_FS_FSTAT64_MS_FN
define_svc_arg2 fs_truncate		FS_FS_TRUNCATE_FN
define_svc_arg2 fs_truncate64		FS_FS_TRUNCATE64_FN
define_svc_arg2 fs_ftruncate		FS_FS_FTRUNCATE_FN
define_svc_arg2 fs_ftruncate64		FS_FS_FTRUNCATE64_FN
define_svc_arg2 fs_utimes		FS_FS_UTIMES_FN
define_svc_arg2 fs_utimes_us		FS_FS_UTIMES_US_FN
define_svc_arg2 fs_utimes_ms		FS_FS_UTIMES_MS_FN
define_svc_arg2 fs_statvfs		FS_FS_STATVFS_FN
define_svc_arg2 fs_fstatvfs		FS_FS_FSTATVFS_FN
define_svc_arg3 _fs_open		FS__FS_OPEN_FN
define_svc_arg4 _fs_lseek		FS__FS_LSEEK_FN
define_svc_arg4 _fs_lseek64		FS__FS_LSEEK64_FN
define_svc_arg3 _fs_ioctl		FS__FS_IOCTL_FN
define_svc_arg3 _fs_fcntl		FS__FS_FCNTL_FN


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
