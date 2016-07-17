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

#include <typedef.h>

#include <bk/memory/access.h>
#include <bk/fs/vfs.h>
#include <bk/utime.h>
#include <bk/uapi/sys/time.h>

#include <tk/timer.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int xutimes(const char *filename, const struct timeval *times);

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
 Funtion	:utime
 Input		:const char *filename
 		 < file path to change its access time and modification time >
 		 const struct utimbuf *times
 		 < access time and modification time to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change file last access and modification times
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int utime(const char *filename, const struct utimbuf *times)
{
	struct timeval timevals[2];
	int err;
	
	
	if (times) {
		err = vm_check_accessR((void*)times, sizeof(struct utimbuf));
		
		if (UNLIKELY(err)) {
			return(-EINVAL);
		}
		
		timevals[0].tv_sec = times->actime;
		timevals[0].tv_usec = 0;
		
		timevals[1].tv_sec = times->modtime;
		timevals[1].tv_usec = 0;
		
		err = xutimes(filename, timevals);
	} else {
		err = xutimes(filename, NULL);
	}
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:utimes
 Input		:const char *filename
 		 < file path to change its access time and modification time >
 		 const struct timeval times[2]
 		 < access time and modification time to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change file last access and modification times
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int utimes(const char *filename, const struct timeval times[2])
{
	int err;
	
	if (times) {
		err = vm_check_accessR((void*)times, sizeof(struct timeval) * 2);
		
		if (UNLIKELY(err)) {
			return(-EINVAL);
		}
	}
	
	err = xutimes(filename, times);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_current_timeval
 Input		:struct timespec *time
 		 < time value to set current time >
 Output		:struct timespec *time
 		 < time value to set current time >
 Return		:void
 Description	:get current time as timeval
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void get_current_timeval(struct timespec *time)
{
	unsigned long msec;
	get_current_time(&time->tv_sec, &msec);
	time->tv_nsec = msec * 1000 * 1000;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:xutimes
 Input		:const char *filename
 		 < file path to change its access time and modification time >
 		 const struct timeval times[2]
 		 < access time and modification time to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change file last access and modification times
==================================================================================
*/
LOCAL int xutimes(const char *filename, const struct timeval *times)
{
	struct file_name *found;
	struct vnode *vnode;
	struct process *proc = get_current();
	struct timespec current_time;
	int err;
	
	if (UNLIKELY(!filename)) {
		return(-EACCES);
	}
	
	err = vfs_lookup(filename, &found, LOOKUP_ENTRY | LOOKUP_FOLLOW_LINK);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	vnode = found->dentry->d_vnode;
	
	put_file_name(found);
	
	if (!times) {
		get_current_timeval(&current_time);
		vnode->v_atime = current_time;
		vnode->v_mtime = current_time;
	} else {
		timeval_to_timespec(&vnode->v_atime, &times[0]);
		timeval_to_timespec(&vnode->v_mtime, &times[1]);
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
