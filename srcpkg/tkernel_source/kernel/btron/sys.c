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
#include <bk/kernel.h>
#include <tk/kernel.h>


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
#define	TRON_NAME		"tron"
#define	DIST_NAME		"*aerial"
#define	SYS_NAME		(TRON_NAME " " DIST_NAME)

//#define	SYS_RELEASE		"0.01"
#define	SYS_RELEASE		"3"


#define	TRON_VERSION		"2.01"
#define	SYS_VERSION		("2015" " " TRON_VERSION)

#define	NODE_NAME_NONE		"(none)"

#define	MACHINE_NAME		"i386"

#define	DOMAIN_NAME_NONE	"(none)"

#define	OLD_OLD_UTS_LEN		8
#define	OLD_UTS_LEN		64
#define	NEW_UTS_LEN		64

struct oldold_utsname {
	char	sysname[OLD_OLD_UTS_LEN + 1];
	char	nodename[OLD_OLD_UTS_LEN + 1];
	char	release[OLD_OLD_UTS_LEN + 1];
	char	version[OLD_OLD_UTS_LEN + 1];
	char	machine[OLD_OLD_UTS_LEN + 1];
};

struct old_utsname {
	char	sysname[OLD_UTS_LEN + 1];	/* operating system name	*/
	char	nodename[OLD_UTS_LEN + 1];	/* name within some 
						implementation-defined network	*/
	char	release[OLD_UTS_LEN + 1];	/* operating system release	*/
	char	version[OLD_UTS_LEN + 1];	/* operating system version	*/
	char	machine[OLD_UTS_LEN + 1];	/* hardware identifier		*/
};

struct new_utsname {
	char	sysname[NEW_UTS_LEN + 1];	/* operating system name	*/
	char	nodename[NEW_UTS_LEN + 1];	/* name within some 	
					   	implementation-defined network	*/
	char	release[NEW_UTS_LEN + 1];	/* operating system release	*/
	char	version[NEW_UTS_LEN + 1];	/* operating system version	*/
	char	machine[NEW_UTS_LEN + 1];	/* hardware identifier		*/
	char	domainname[NEW_UTS_LEN + 1];	/* nis or yp domain name	*/
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct oldold_utsname tron_old_utsname =
{
	.sysname = TRON_NAME,
	.nodename = NODE_NAME_NONE,
	.release = SYS_RELEASE,
	.version = TRON_VERSION,
	.machine = MACHINE_NAME,
};

LOCAL struct new_utsname tron_utsname =
{
//	.sysname = SYS_NAME,
	.sysname = "Linux",
	.nodename = NODE_NAME_NONE,
	.release = SYS_RELEASE,
	.version = SYS_VERSION,
	.machine = MACHINE_NAME,
	.domainname = DOMAIN_NAME_NONE,
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:uname
 Input		:struct new_utsname *buf
 		 < a user buffer of name and information about the kernel >
 Output		:void
 Return		:int
 		 < result >
 Description	:new uname
 		 get name and information about current kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int uname(struct new_utsname *buf)
{
	if (!buf) {
		return(-EFAULT);
	}
	
	if (copy_to_user(buf, &tron_utsname, sizeof(struct new_utsname))) {
#if 0
	printf("sysname : %s\n", buf->sysname);
	printf("nodename : %s\n", buf->nodename);
	printf("release : %s\n", buf->release);
	printf("version : %s\n", buf->version);
	printf("machine : %s\n", buf->machine);
	printf("domainname: %s\n", buf->domainname);
#endif
		return(E_OK);
	}
	
	return(-EFAULT);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:olduname
 Input		:struct old_utsname *buf
 		 < a user buffer of name and information about the kernel >
 Output		:void
 Return		:int
 		 < result >
 Description	:old uname
 		 get name and information about current kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int olduname(struct old_utsname *buf)
{
	int err;
	
	if (!buf) {
		return(-EFAULT);
	}
	
	err = ChkSpaceRW(buf, sizeof(struct old_utsname));
	
	if (err) {
		return(-EFAULT);
	}
	
	memcpy((void*)buf->sysname,
			(void*)tron_utsname.sysname, OLD_UTS_LEN + 1);
	memcpy((void*)buf->nodename,
			(void*)tron_utsname.nodename, OLD_UTS_LEN + 1);
	memcpy((void*)buf->release,
			(void*)tron_utsname.release, OLD_UTS_LEN + 1);
	memcpy((void*)buf->version,
			(void*)tron_utsname.version, OLD_UTS_LEN + 1);
	memcpy((void*)buf->machine,
			(void*)tron_utsname.machine,OLD_UTS_LEN + 1);
	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:oldolduname
 Input		:struct oldold_utsname *buf
 		 < a user buffer of name and information about the kernel >
 Output		:void
 Return		:int
 		 < result >
 Description	:old old uname
 		 get name and information about current kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int oldolduname(struct oldold_utsname *buf)
{
	if (!buf) {
		return(-EFAULT);
	}
	
	if (copy_to_user(buf, &tron_old_utsname, sizeof(struct oldold_utsname))) {
		return(E_OK);
	}
	
	return(-EFAULT);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:gettid
 Input		:void
 Output		:void
 Return		:pid_t
 		 < thread id >
 Description	:get thread id
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t gettid(void)
{
	struct process *proc = get_current();
	struct task *task = get_current_task();
	
	printf("gettid");
	
	if (proc->group_leader == task ) {
		printf("[tid=%d]\n", proc->pid);
		return(proc->pid);
	}
	
	printf("[tid=%d]\n", task->tskid);
	return(task->tskid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:geteuid32
 Input		:void
 Output		:void
 Return		:uid_t
 		 < effective user id >
 Description	:returns the effective user id of the calling process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL uid_t geteuid32(void)
{
	//printf("geteuid32[%d]\n", get_current()->euid);
	return(get_current()->euid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getuid32
 Input		:void
 Output		:void
 Return		:uid_t
 		 < real user id >
 Description	:returns the real user id of the calling process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL uid_t getuid32(void)
{
	//printf("getuid32[%d]\n", get_current()->uid);
	return(get_current()->uid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getegid32
 Input		:void
 Output		:void
 Return		:gid_t
 		 < effective group id >
 Description	:returns the effective group id of the calling process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL uid_t getegid32(void)
{
	//printf("getegid32[%d]\n", get_current()->egid);
	return(get_current()->egid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getgid32
 Input		:void
 Output		:void
 Return		:gid_t
 		 < real group id >
 Description	:returns the real group id of the calling process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL uid_t getgid32(void)
{
	//printf("getgid32[%d]\n", get_current()->gid);
	return(get_current()->gid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_machine_name
 Input		:void
 Output		:void
 Return		:char *
 		 < machine name >
 Description	:get machine name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT char* get_machine_name(void)
{
	return(tron_utsname.machine);
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
