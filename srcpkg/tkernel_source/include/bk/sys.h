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

#ifndef	__BK_SYS_H__
#define	__BK_SYS_H__

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct new_utsname;
struct old_utsname;
struct oldold_utsname;

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
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
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
SYSCALL int uname(struct new_utsname *buf);

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
SYSCALL int olduname(struct old_utsname *buf);

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
SYSCALL int oldolduname(struct oldold_utsname *buf);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getpid
 Input		:void
 Output		:void
 Return		:pid_t
 		 < pid of current process >
 Description	:get pid of current process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t getpid(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getppid
 Input		:void
 Output		:void
 Return		:pid_t
 		 < pid of parent process >
 Description	:get pid of parent process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t getppid(void);

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
SYSCALL pid_t gettid(void);

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
SYSCALL uid_t geteuid32(void);

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
SYSCALL uid_t getuid32(void);

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
SYSCALL uid_t getegid32(void);

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
SYSCALL uid_t getgid32(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:restart_syscall
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:restart a system call after interruption by a stop
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int restart_syscall(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:times
 Input		:struct tms *buf
 		 < times buffer to out put >
 Output		:struct tms *buf
 		 < times buffer to out put >
 Return		:clock_t
 		 < result >
 Description	:get proces times
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL clock_t times(struct tms *buf);

/*
----------------------------------------------------------------------------------
	kernel internal operations
----------------------------------------------------------------------------------
*/
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
EXPORT char* get_machine_name(void);

#endif	// __BK_SYS_H__
