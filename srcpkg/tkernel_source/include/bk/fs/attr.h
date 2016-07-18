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

#ifndef	__BK_FS_ATTR_H__
#define	__BK_FS_ATTR_H__


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
----------------------------------------------------------------------------------
	file attribute
----------------------------------------------------------------------------------
*/
struct iattr {
	unsigned int	ia_valid;
	umode_t		ia_mode;
	uid_t		ia_uid;
	gid_t		ia_gid;
	loff_t		ia_size;
	struct timespec	ia_atime;
	struct timespec	ia_mtime;
	struct timespec	ia_ctime;
	
	/* file system must check (ia_valid & ATTR_FILE) and (!ia_file)		*/
	struct fiel	*ia_file;
};

#define	ATTR_MODE	0x00000001
#define	ATTR_UID	0x00000002
#define	ATTR_GID	0x00000004
#define	ATTR_SIZE	0x00000008
#define	ATTR_ATIME	0x00000010
#define	ATTR_MTIME	0x00000020
#define	ATTR_CTIME	0x00000040
#define	ATTR_ATIME_SET	0x00000080
#define	ATTR_MTIME_SET	0x00000100
#define	ATTR_FORCE	0x00000200	/* not a change, but a change it	*/
#define	ATTR_ATTR_FLAG	0x00000400
#define	ATTR_KILL_SUID	0x00000800
#define	ATTR_KILL_SGID	0x00001000
#define	ATTR_FILE	0x00002000
#define	ATTR_KILL_PRIV	0x00004000
#define	ATTR_OPEN	0x00008000
#define	ATTR_TIMES_SET	0x00010000

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
 Funtion	:chmod
 Input		:const char *pathname
 		 < path name to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chmod(const char *pathname, mode_t mode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchmod
 Input		:int fd
 		 < open file descriptor to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fchmod(int fd, mode_t mode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchmodat
 Input		:int dirfd
 		 < directory open file descriptor >
 		 const char *pathname
 		 < path name to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 		 int flags
 		 < flags >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:chown
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chown(const char *pathname, uid_t owner, gid_t group);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:chown32
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chown32(const char *pathname, uid_t owner, gid_t group)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchown32
 Input		:int fd
 		 < open file descriptor >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fchown32(int fd, uid_t owner, gid_t group);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lchown32
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int lchown32(const char *pathname, uid_t owner, gid_t group);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchownat
 Input		:int dirfd
 		 < open directory file descriptor >
 		 const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 		 int flags
 		 < flags >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int
fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);

#endif	// __BK_FS_ATTR_H__
