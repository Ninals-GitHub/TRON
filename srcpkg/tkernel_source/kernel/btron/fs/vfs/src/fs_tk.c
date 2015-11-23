/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/01/08.
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
 *	@(#)fs_tk.c
 *
 */

#include "fsdefs.h"
#include <tk/syslib.h>
#include <tk/util.h>
#include <tk/errno.h>
#include <t2ex/ssid.h>

IMPORT ER ChkT2EXLevel( void );

/* Definition of SVC functions */
#include "iffs.h"
#include "fnfs.h"

/*
 *  File System Global Data : Global variable
 *
 *	Generally, user context is required for each process.
 *	T2EX does not support process, therefore, only one user context exists.
 */
LOCAL	struct {
	fs_ctx_t	*uctx;		/* User context of file system	*/
	INT		n_tsd;		/* Number of tsd		*/
	fs_tsd_t	*tsd;		/* Task specific data [n_tsd]	*/
	UW		*fimptsd;	/* Fimp task specific data [n_tsd] */
	FastMLock	mlock;		/* Mlock for exclusive lock	*/
	INT		initialized;	/* Initialized flag		*/
} fs_gdt;

#define	SYS_FSCTX	(fs_gdt.uctx)	/* System FS context		*/

/* ========================================================================= */
/*
 *  Check whether the task exists
 */
EXPORT INT fs_task_exist(ID tid)
{
	T_RTSK	rtsk;

	/* Error code is E_xxx */
	return tk_ref_tsk(tid, &rtsk);
}

/*
 *  Read system configuration (SYSCONF) value
 */
LOCAL	INT	read_config(const B *key, INT base, INT defval)
{
	INT	num;

	if (tk_get_cfn((UB *)key, &num, 1) < 1) return defval;
	return num + base;
}

/*
 *  Error code conversion
 */
LOCAL	ER	E_to_EX(ER ercd)
{
	switch (ercd) {
	case E_OK:	ercd = 0;		break;
	case E_ID:	ercd = EX_INVAL;	break;
	case E_NOMEM:	ercd = EX_NOMEM;	break;
	case E_RSATR:	ercd = EX_INVAL;	break;
	case E_PAR:	ercd = EX_INVAL;	break;
	case E_OBJ:	ercd = EX_AGAIN;	break;
	case E_NOEXS:	ercd = EX_NOENT;	break;
	default:	ercd = EX_NOSYS;
	}
	return ercd;
}

/*
 *  fs_lock : 0 <= locknum < 32
 */
EXPORT	void	fs_lock_lock(INT locknum)
{
	MLock(&fs_gdt.mlock, locknum);
}
EXPORT	void	fs_lock_unlock(INT locknum)
{
	MUnlock(&fs_gdt.mlock, locknum);
}

/* ========================================================================= */

#define	FnNo(n)	((n) >> 16)

/*
 *  Execute Each SVC functions
 */
LOCAL	INT	exe_svc_func(fs_env_t *env, INT fno, const void *par)
{
	INT	err, sts, len;

	err = sts = 0;
	switch(FnNo(fno)) {
	case FnNo(FS_FS_REGIST_FN):
	{	FS_FS_REGIST_PARA *p = (FS_FS_REGIST_PARA *)par;

		if (	p->fimp == NULL ||
			p->fimp->registfn == NULL ||
			p->fimp->unregistfn == NULL ||
			p->fimp->attachfn == NULL ||
			p->fimp->detachfn == NULL ||
			p->fimp->reqfn == NULL ||
			p->fimpnm == NULL ||
			ChkSpaceBstrR((void *)p->fimpnm, 0) <= 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_regist(env, p->fimpnm, p->fimp, p->info,
							fs_gdt.fimptsd );
		}
		break;
	}
	case FnNo(FS_FS_UNREGIST_FN):
	{	FS_FS_UNREGIST_PARA *p = (FS_FS_UNREGIST_PARA *)par;

		if (p->fimpnm == NULL ||
			ChkSpaceBstrR((void *)p->fimpnm, 0) <= 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_unregist(env, p->fimpnm);
		}
		break;
	}
	case FnNo(FS_FS_ATTACH_FN):
	{	FS_FS_ATTACH_PARA *p = (FS_FS_ATTACH_PARA *)par;

		if (p->connm == NULL ||
			ChkSpaceBstrR((void *)p->connm, 0) <= 0 ||
			(p->devnm != NULL &&
				ChkSpaceBstrR((void *)p->devnm, 0) <= 0) ||
			(p->fimpnm != NULL &&
				ChkSpaceBstrR((void *)p->fimpnm, 0) <= 0)) {
			err = EX_INVAL;
		} else {
			sts = xfs_attach(env, p->devnm, p->connm,
					p->fimpnm, p->flags, p->info);
		}
		break;
	}
	case FnNo(FS_FS_DETACH_FN):
	{	FS_FS_DETACH_PARA *p = (FS_FS_DETACH_PARA *)par;

		if (p->connm == NULL ||
			ChkSpaceBstrR((void *)p->connm, 0) <= 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_detach(env, p->connm, 0);
		}
		break;
	}
	case FnNo(FS_FS_CHDIR_FN):
	{	FS_FS_CHDIR_PARA *p = (FS_FS_CHDIR_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_chdir(env, p->path);
		}
		break;
	}
	case FnNo(FS_FS_FCHDIR_FN):
	{	FS_FS_FCHDIR_PARA *p = (FS_FS_FCHDIR_PARA *)par;

		sts = xfs_fchdir(env, p->fd);
		break;
	}
	case FnNo(FS_FS_GETCWD_FN):
	{	FS_FS_GETCWD_PARA *p = (FS_FS_GETCWD_PARA *)par;

		if (p->size == 0 ||
			ChkSpaceRW(p->buf, (INT)p->size) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_getcwd(env, p->buf, p->size);
		}
		break;
	}
	case FnNo(FS_FS_CHMOD_FN):
	{	FS_FS_CHMOD_PARA *p = (FS_FS_CHMOD_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_chmod(env, p->path, p->mode);
		}
		break;
	}
	case FnNo(FS_FS_FCHMOD_FN):
	{	FS_FS_FCHMOD_PARA *p = (FS_FS_FCHMOD_PARA *)par;

		sts = xfs_fchmod(env, p->fd, p->mode);
		break;
	}
	case FnNo(FS_FS_FSYNC_FN):
	{	FS_FS_FSYNC_PARA *p = (FS_FS_FSYNC_PARA *)par;

		sts = xfs_fsync(env, p->fd);
		break;
	}
	case FnNo(FS_FS_FDATASYNC_FN):
	{	FS_FS_FDATASYNC_PARA *p = (FS_FS_FDATASYNC_PARA *)par;

		sts = xfs_fdatasync(env, p->fd);
		break;
	}
	case FnNo(FS_FS_SYNC_FN):
	{
		sts = xfs_sync(env);
		break;
	}
	case FnNo(FS_FS_MKDIR_FN):
	{	FS_FS_MKDIR_PARA *p = (FS_FS_MKDIR_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_mkdir(env, p->path, p->mode);
		}
		break;
	}
	case FnNo(FS_FS_RMDIR_FN):
	{	FS_FS_RMDIR_PARA *p = (FS_FS_RMDIR_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_rmdir(env, p->path);
		}
		break;
	}
	case FnNo(FS_FS_GETDENTS_FN):
	{	FS_FS_GETDENTS_PARA *p = (FS_FS_GETDENTS_PARA *)par;

		if (p->buf == NULL || p->bufsz <= 0 ||
			ChkSpaceRW(p->buf, (INT)p->bufsz) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_getdents(env, p->fd, p->buf, p->bufsz);
		}
		break;
	}
	case FnNo(FS_FS_READ_FN):
	{	FS_FS_READ_PARA *p = (FS_FS_READ_PARA *)par;

		if (p->count != 0 &&
			ChkSpaceRW(p->buf, (INT)p->count) < 0) {
			printf("p->count:%d ", p->count);
			printf("p->buf:0x%08X\n", p->buf);
			//printf("ChkSpaceRW:%d\n", ChkSpaceRW(p->buf, (INT)p->count));
			err = EX_INVAL;
		} else {
			sts = xfs_read(env, p->fd, p->buf, p->count);
		}
		break;
	}
	case FnNo(FS_FS_WRITE_FN):
	{	FS_FS_WRITE_PARA *p = (FS_FS_WRITE_PARA *)par;

		if (p->count != 0 &&
			ChkSpaceR((void *)p->buf, (INT)p->count) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_write(env, p->fd, p->buf, p->count);
		}
		break;
	}
	case FnNo(FS_FS_CLOSE_FN):
	{	FS_FS_CLOSE_PARA *p = (FS_FS_CLOSE_PARA *)par;

		if (p->fd <= 2) {
			/* 0,1,2 (stdin,stdout,stderr) must not be closed */
			err = (p->fd < 0) ? EX_BADF : EX_PERM;
		} else {
			sts = xfs_close(env, p->fd);
		}
		break;
	}
	case FnNo(FS_FS_RENAME_FN):
	{	FS_FS_RENAME_PARA *p = (FS_FS_RENAME_PARA *)par;

		if (ChkSpaceBstrR((void *)p->oldpath, 0) < 0 ||
			ChkSpaceBstrR((void *)p->newpath, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_rename(env, p->oldpath, p->newpath);
		}
		break;
	}
	case FnNo(FS_FS_UNLINK_FN):
	{	FS_FS_UNLINK_PARA *p = (FS_FS_UNLINK_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_unlink(env, p->path);
		}
		break;
	}
	case FnNo(FS_FS_STAT_FN):
	{	FS_FS_STAT_PARA *p = (FS_FS_STAT_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->buf, sizeof(*p->buf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_stat(env, p->path, p->buf);
		}
		break;
	}
	case FnNo(FS_FS_STAT_US_FN):
	{	FS_FS_STAT_US_PARA *p = (FS_FS_STAT_US_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->ubuf, sizeof(*p->ubuf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_stat_us(env, p->path, p->ubuf);
		}
		break;
	}
	case FnNo(FS_FS_STAT_MS_FN):
	{	FS_FS_STAT_MS_PARA *p = (FS_FS_STAT_MS_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->mbuf, sizeof(*p->mbuf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_stat_ms(env, p->path, p->mbuf);
		}
		break;
	}
	case FnNo(FS_FS_STAT64_FN):
	{	FS_FS_STAT64_PARA *p = (FS_FS_STAT64_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->buf64, sizeof(*p->buf64)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_stat64(env, p->path, p->buf64);
		}
		break;
	}
	case FnNo(FS_FS_STAT64_US_FN):
	{	FS_FS_STAT64_US_PARA *p = (FS_FS_STAT64_US_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->ubuf64, sizeof(*p->ubuf64)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_stat64_us(env, p->path, p->ubuf64);
		}
		break;
	}
	case FnNo(FS_FS_STAT64_MS_FN):
	{	FS_FS_STAT64_MS_PARA *p = (FS_FS_STAT64_MS_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->mbuf64, sizeof(*p->mbuf64)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_stat64_ms(env, p->path, p->mbuf64);
		}
		break;
	}
	case FnNo(FS_FS_FSTAT_FN):
	{	FS_FS_FSTAT_PARA *p = (FS_FS_FSTAT_PARA *)par;

		if (ChkSpaceRW(p->buf, sizeof(*p->buf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstat(env, p->fd, p->buf);
		}
		break;
	}
	case FnNo(FS_FS_FSTAT_US_FN):
	{	FS_FS_FSTAT_US_PARA *p = (FS_FS_FSTAT_US_PARA *)par;

		if (ChkSpaceRW(p->ubuf, sizeof(*p->ubuf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstat_us(env, p->fd, p->ubuf);
		}
		break;
	}
	case FnNo(FS_FS_FSTAT_MS_FN):
	{	FS_FS_FSTAT_MS_PARA *p = (FS_FS_FSTAT_MS_PARA *)par;

		if (ChkSpaceRW(p->mbuf, sizeof(*p->mbuf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstat_ms(env, p->fd, p->mbuf);
		}
		break;
	}
	case FnNo(FS_FS_FSTAT64_FN):
	{	FS_FS_FSTAT64_PARA *p = (FS_FS_FSTAT64_PARA *)par;

		if (ChkSpaceRW(p->buf64, sizeof(*p->buf64)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstat64(env, p->fd, p->buf64);
		}
		break;
	}
	case FnNo(FS_FS_FSTAT64_US_FN):
	{	FS_FS_FSTAT64_US_PARA *p = (FS_FS_FSTAT64_US_PARA *)par;

		if (ChkSpaceRW(p->ubuf64, sizeof(*p->ubuf64)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstat64_us(env, p->fd, p->ubuf64);
		}
		break;
	}
	case FnNo(FS_FS_FSTAT64_MS_FN):
	{	FS_FS_FSTAT64_MS_PARA *p = (FS_FS_FSTAT64_MS_PARA *)par;

		if (ChkSpaceRW(p->mbuf64, sizeof(*p->mbuf64)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstat64_ms(env, p->fd, p->mbuf64);
		}
		break;
	}
	case FnNo(FS_FS_TRUNCATE_FN):
	{	FS_FS_TRUNCATE_PARA *p = (FS_FS_TRUNCATE_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_truncate64(env, p->path, (off64_t)p->length);
		}
		break;
	}
	case FnNo(FS_FS_TRUNCATE64_FN):
	{	FS_FS_TRUNCATE64_PARA *p = (FS_FS_TRUNCATE64_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_truncate64(env, p->path, p->length64);
		}
		break;
	}
	case FnNo(FS_FS_FTRUNCATE_FN):
	{	FS_FS_FTRUNCATE_PARA *p = (FS_FS_FTRUNCATE_PARA *)par;

		sts = xfs_ftruncate64(env, p->fd, (off64_t)p->length);
		break;
	}
	case FnNo(FS_FS_FTRUNCATE64_FN):
	{	FS_FS_FTRUNCATE64_PARA *p = (FS_FS_FTRUNCATE64_PARA *)par;

		sts = xfs_ftruncate64(env, p->fd, p->length64);
		break;
	}
	case FnNo(FS_FS_UTIMES_FN):
	{	FS_FS_UTIMES_PARA *p = (FS_FS_UTIMES_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
			break;
		}
		if (p->tim != NULL) {
			if (ChkSpaceRW((void *)p->tim,
				sizeof(struct timeval) * 2) < 0) {
				err = EX_INVAL;
				break;
			}
			if (	p->tim[0].tv_sec < 0 ||
				p->tim[0].tv_usec < 0 ||
				p->tim[0].tv_usec >= 1000000 ||
				p->tim[1].tv_sec < 0 ||
				p->tim[1].tv_usec < 0 ||
				p->tim[1].tv_usec >= 1000000) {
				err = EX_INVAL;
				break;
			}
		}
		sts = xfs_utimes(env, p->path, p->tim);
		break;
	}
	case FnNo(FS_FS_UTIMES_US_FN):
	{	FS_FS_UTIMES_US_PARA *p = (FS_FS_UTIMES_US_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			(p->tim_u != NULL && ( ChkSpaceRW((void *)p->tim_u,
				sizeof(SYSTIM_U) * 2) < 0 ||
				p->tim_u[0] < 0 || p->tim_u[1] < 0) ) ) {
			err = EX_INVAL;
			break;
		}
		sts = xfs_utimes_us(env, p->path, p->tim_u);
		break;
	}
	case FnNo(FS_FS_UTIMES_MS_FN):
	{	FS_FS_UTIMES_MS_PARA *p = (FS_FS_UTIMES_MS_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			(p->tim_m != NULL && ChkSpaceRW((void *)p->tim_m,
				sizeof(SYSTIM) * 2) < 0) ) {
			err = EX_INVAL;
			break;
		}
		sts = xfs_utimes_ms(env, p->path, p->tim_m);
		break;
	}
	case FnNo(FS_FS_STATVFS_FN):
	{	FS_FS_STATVFS_PARA *p = (FS_FS_STATVFS_PARA *)par;

		if (ChkSpaceBstrR((void *)p->path, 0) < 0 ||
			ChkSpaceRW(p->buf, sizeof(*p->buf)) < 0 ) {
			err = EX_INVAL;
		} else {
			sts = xfs_statvfs(env, p->path, p->buf);
		}
		break;
	}
	case FnNo(FS_FS_FSTATVFS_FN):
	{	FS_FS_FSTATVFS_PARA *p = (FS_FS_FSTATVFS_PARA *)par;

		if (ChkSpaceRW(p->buf, sizeof(*p->buf)) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_fstatvfs(env, p->fd, p->buf);
		}
		break;
	}
	case FnNo(FS__FS_OPEN_FN):
	{	FS__FS_OPEN_PARA *p = (FS__FS_OPEN_PARA *)par;
		if (ChkSpaceBstrR((void *)p->path, 0) < 0) {
			err = EX_INVAL;
		} else {
			sts = xfs_open(env, p->path, p->oflag, p->mode);
		}
		break;
	}
	case FnNo(FS__FS_LSEEK_FN):
	{	FS__FS_LSEEK_PARA *p = (FS__FS_LSEEK_PARA *)par;

		sts = xfs_lseek(env, p->fd, p->offset, p->whence, p->roff);
		break;
	}
	case FnNo(FS__FS_LSEEK64_FN):
	{	FS__FS_LSEEK64_PARA *p = (FS__FS_LSEEK64_PARA *)par;

		sts = xfs_lseek64(env, p->fd, p->offset64, p->whence, p->roff);
		break;
	}
	case FnNo(FS__FS_IOCTL_FN):
	{	FS__FS_IOCTL_PARA *p = (FS__FS_IOCTL_PARA *)par;

		if (p->arg != NULL) {
			len = (p->request >> IOCPARM_SHIFT) & IOCPARM_MASK;
			if (len > 0 && (
				((p->request & IOC_OUT) != 0 &&
					ChkSpaceRW(p->arg, len) < 0) ||
				((p->request & IOC_IN) != 0 &&
					ChkSpaceR(p->arg, len) < 0)) ) {
				err = EX_INVAL;
				break;
			}
		}
		sts = xfs_ioctl(env, p->fd, p->request, p->arg);
		break;
	}
	case FnNo(FS__FS_FCNTL_FN):
	{	FS__FS_FCNTL_PARA *p = (FS__FS_FCNTL_PARA *)par;

		if (p->arg != NULL) {
			len = (p->cmd >> FCTLPARM_SHIFT) & FCTLPARM_MASK;
			if (len > 0 && (
				((p->cmd & FCTL_OUT) != 0 &&
					ChkSpaceRW(p->arg, len) < 0) ||
				((p->cmd & FCTL_IN) != 0 &&
					ChkSpaceR(p->arg, len) < 0)) ) {
				err = EX_INVAL;
				break;
			}
		}
		sts = xfs_fcntl(env, p->fd, p->cmd, p->arg);
		break;
	}
	default:	/* Invalid SVC function number */
		err = EX_NOSYS;		/* E_RSFN */
	}

	return (err < 0) ? err : ((sts < 0) ? env->t_errno : sts);
}

/*
 *  Finalize file system global data (fs_gdt)
 */

LOCAL	void	fs_gdt_finalize(void)
{
	/* Free SYS_FSCTX */
	if (SYS_FSCTX != NULL) {
		fs_free(SYS_FSCTX);
		SYS_FSCTX = NULL;
	}

	/* Free tsd */
	if (fs_gdt.tsd != NULL) {
		fs_free(fs_gdt.tsd);
	}

	/* Delete mlock */
	DeleteMLock(&fs_gdt.mlock);

	/* Clear gdt */
	memset(&fs_gdt, 0, sizeof(fs_gdt));
}

/*
 *  Initialize file system global data (fs_gdt)
 */
LOCAL	INT	fs_gdt_initialize(INT maxfile)
{
	INT	sts;

	/* Clear gdt */
	(void)memset(&fs_gdt, 0, sizeof(fs_gdt));
	sts = EX_NOMEM;

	/* Allocate system context (ctx) */
	SYS_FSCTX = fs_malloc(FS_CTX_SIZE(maxfile));
	if (SYS_FSCTX == NULL) goto exit0;

	/* Allocate task specific data (tsd) */
	fs_gdt.n_tsd = read_config("TMaxTskId", 0, 0);
	if (fs_gdt.n_tsd <= 0) {
		sts = EX_NOSYS;
		goto exit0;
	}
	fs_gdt.tsd = (fs_tsd_t *)fs_malloc(
			(sizeof(fs_tsd_t) + sizeof(UW)) * fs_gdt.n_tsd);
	if (fs_gdt.tsd == NULL) goto exit0;
	fs_gdt.fimptsd = (UW*)((UB*)fs_gdt.tsd + 
					(sizeof(fs_tsd_t) * fs_gdt.n_tsd));

	/* Clear tsd & fimptsd */
	memset(fs_gdt.tsd, 0, (sizeof(fs_tsd_t) + sizeof(UW)) * fs_gdt.n_tsd);

	/* Create mlock */
	sts = CreateMLock(&fs_gdt.mlock, (UB*)"Fsys");
exit0:
	return sts;
}

/*
 *  Get Task Specifiec Data (TSD)
 */
EXPORT	fs_tsd_t	*fs_tsd_get(ID tskid)
{
	if (tskid == TSK_SELF) {
		tskid = tk_get_tid();
	}
	if (fs_gdt.tsd != NULL && tskid > 0 && tskid <= fs_gdt.n_tsd) {
		return &fs_gdt.tsd[tskid - 1];
	}
	return NULL;
}

/*
 *  Setup environment for FS access
 */
LOCAL	void	env_setup(fs_env_t *env, fs_ctx_t *ctx)
{
	/* Get task id and tsd */
	env->t_tid = tk_get_tid();
	env->t_tsd = (fs_tsd_t *)fs_tsd_get(env->t_tid);
	env->t_ctx = ctx;
}

/*
 *  File System SVC handler
 */
LOCAL	ER	fs_svchdr(const void *para, W fn)
{
	ER		ercd;
	fs_env_t	env;

	ercd = ChkT2EXLevel();
	if (ercd < E_OK) {
		return ercd;
	}

	if (FnNo(fn) == FnNo(FS_FS_BREAK_FN)) {
		/* "fs_break" may be called in task independent part */
		ercd = xfs_break(((FS_FS_BREAK_PARA *)para)->tskid);

	} else if (tk_get_rid(TSK_SELF) < E_OK) {
		/* Called in a task independent part */
		ercd = EX_NOSYS;

	} else {
		/* Call in a task */
		env_setup(&env, SYS_FSCTX);

		/* Execute svc function */
		env.t_tsd->t_stat.c.exec_fs = 1;
		ercd = (ER)exe_svc_func(&env, fn, para);
		env.t_tsd->t_stat.c.exec_fs = 0;

		/* Clear break status of fimp */
		if (env.t_tsd->t_stat.c.break_called != 0) {
			xfs_break_impl(env.t_tsd->t_con, env.t_tid, FALSE);
		}
		env.t_tsd->t_stat.all = 0;
	}
	return ercd;
}

/*
 *  Initialized or not
 */
EXPORT	INT	fs_tk_is_initialized(void)
{
	return fs_gdt.initialized;
}

/*
 *  Finalize file system function
 */
EXPORT	INT	fs_tk_finalize(void)
{
	fs_env_t	env;

	(void)tk_def_ssy(FS_SVC, 0);

	env_setup(&env, SYS_FSCTX);
	fs_ctx_finalize(SYS_FSCTX, &env);
	fs_fini(&env);
	fs_gdt_finalize();
	fs_gdt.initialized = 0;
	return 0;
}

/*
 *  Initialize file system function
 */
EXPORT	INT	fs_tk_initialize(void)
{
	INT		sts;
	fs_config_t	config;
	fs_env_t	env;
	T_DSSY		dssy;

	/* Get system configurations & check it */
	config.c_maxfile = read_config("FsMaxFile",
				BASE_FS_OPEN_MAX, DEF_FS_OPEN_MAX);
	config.c_maxfimp = read_config("FsMaxFIMP",
				BASE_FS_FIMP_MAX, DEF_FS_FIMP_MAX);
	config.c_maxcon = read_config("FsMaxCON",
				BASE_FS_CON_MAX, DEF_FS_CON_MAX);
	if (	config.c_maxfile < MIN_FS_OPEN_MAX ||
		config.c_maxfimp < MIN_FS_FIMP_MAX ||
		config.c_maxcon < MIN_FS_CON_MAX) {
		sts = EX_INVAL;
		goto exit0;
	}

	/* Initailize file system global data */
	sts = fs_gdt_initialize(config.c_maxfile);
	if (sts < 0) goto exit1;

	/* Initialize fs context */
	sts = fs_ctx_initialize(SYS_FSCTX, config.c_maxfile);
	if (sts < 0) goto exit1;

	/* Initialize file system */
	env_setup(&env, SYS_FSCTX);
	sts = fs_init(&env, &config);
	if (sts < 0) goto exit1;

	/* Define file system subsystem */
	dssy.ssyatr = TA_RNG0;
	dssy.ssypri = FS_PRI;
	dssy.svchdr = (FP)fs_svchdr;
	dssy.breakfn = NULL;
	dssy.startupfn = NULL;
	dssy.cleanupfn = NULL;
	dssy.eventfn = NULL;
	dssy.resblksz = 0;
	sts = tk_def_ssy(FS_SVC, &dssy);
	sts = E_to_EX(sts);
	if (sts < 0) goto exit2;

	fs_gdt.initialized = 1;
	goto exit0;
exit2:
	fs_fini(&env);
exit1:
	fs_gdt_finalize();
exit0:
	return sts;
}

/*
 *  Call FIMP service
 */
EXPORT	INT	fs_callfimp(fs_env_t *env, fs_cond_t *con, INT rcode)
{
	fimp_t		*cmd;
	fs_tsd_t	*tsd;
	INT		sts;

	sts = EX_INTR;
	tsd = env->t_tsd;

	if (tsd->t_stat.c.break_done == 0) {
		tsd->t_con = con;

		/* Call FIMP */
		tsd->t_stat.c.exec_fimp = 1;
		if (tsd->t_stat.c.break_done == 0) {
			/* Set common fields */
			cmd = &env->t_cmd;
			cmd->com.r_code = rcode;
			cmd->com.coninf = &con->c_coninf;
			sts = (INT)con->c_fimpsd->p_fimp.reqfn(cmd);
		}
		tsd->t_stat.c.exec_fimp = 0;

		if (tsd->t_stat.c.break_called != 0) {
			tsd->t_stat.c.break_done = 1;
		}
	}
	return sts;
}


