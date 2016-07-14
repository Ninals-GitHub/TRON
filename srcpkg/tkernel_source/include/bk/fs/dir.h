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

#ifndef	__BK_FS_DIR_H__
#define	__BK_FS_DIR_H__

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
 Funtion	:dent64_reclen
 Input		:unsigned short name_len
 		 < name length of a directory entry>
 Output		:void
 Return		:unsigned short
 		 < d_reclen >
 Description	:calculate reclen of linux_dirent64
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	DENT64_PAD		4
#define	DENT64_ROUND		(DENT64_PAD - 1)
#define dent64_reclen(name_len) (((name_len + 1) + sizeof(ino64_t) +		\
						sizeof(loff_t) +		\
						sizeof(unsigned short) +	\
						sizeof(unsigned char) +		\
						DENT64_ROUND) & ~DENT64_ROUND)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dent64_offset
 Input		:offset
 		 < offset of a current dent64 entry >
 		 unsigned short d_reclen
 		 < reclen of a current dent64 entry >
 Output		:void
 Return		:loff_t
 		 < offset of next dent64 entry >
 Description	:calculate next offset of linux_dirent64
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	dent64_offset(offset, d_reclen) ((offset) + (d_reclen) - sizeof(ino64_t))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dent64_next
 Input		:struct linux_dirent64 *dirp
 		 < current dent64 entry >
 		 unsigned short d_reclen
 		 < reclen of a current dent64 entry >
 Output		:void
 Return		:struct linux_dirent64*
 		 < next dent64 entry >
 Description	:get next dent64 entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE struct linux_dirent64*
dent64_next(struct linux_dirent64 *dirp, unsigned short d_reclen)
{
	return((struct linux_dirent64*)(((char*)dirp) + d_reclen));
}
#if 0
#define	dent64_next(dirp, d_reclen) ((struct linux_dent64*)(((char*)dirp) +	\
								(d_reclen)))
#endif
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_file_type
 Input		:struct dentry *dentry
 		 < dentry to get its file type >
 Output		:void
 Return		:unsigned char
 		 < a file type >
 Description	:get file type from a dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT unsigned char dentry_file_type(struct dentry *dentry);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_dir_path
 Input		:const char *pathname
 		 < directory path name to show >
 Output		:void
 Return		:void
 Description	:show sub directory of the pathname
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void show_dir_path(const char *pathname);

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getdents64
 Input		:int fd
 		 < open file descriptor >
 		 struct linux_dirent64 *dirp
 		 < directory entries for linux >
 		 unsigned int count
 		 < size of a buffer >
 Output		:struct linux_dirent *dirp
 		 < directory entries for linux >
 Return		:int
 		 < result >
 Description	:get directory entries
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int getdents64(int fd, struct linux_dirent64 *dirp, unsigned int count);

#endif	// __BK_FS_DIR_H__
