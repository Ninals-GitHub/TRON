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

#ifndef	__BK_FS_VFS_H__
#define	__BK_FS_VFS_H__

#include <bk/fs/dentry.h>
#include <bk/fs/file_system_type.h>
#include <bk/fs/mount.h>
#include <bk/fs/super_block.h>
#include <bk/fs/vnode.h>
#include <bk/fs/path.h>
#include <bk/fs/file.h>
#include <bk/fs/block_device.h>
#include <bk/fs/character_device.h>
#include <bk/fs/page_cache.h>

#include <bk/fs/ramfs/initramfs.h>

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
	file types
----------------------------------------------------------------------------------
*/
#define	DT_UNKNOWN	0
#define	DT_FIFO		1		// a fifo
#define	DT_CHR		2		// a character device
#define	DT_DIR		4		// a directory
#define	DT_BLK		6		// a block device
#define	DT_REG		8		// a regular file
#define	DT_LNK		10		// a symbolic link
#define	DT_SOCK		12		// a socket
#define	DT_WHT		14		// whiteout

/*
----------------------------------------------------------------------------------
	directory entries for linux
----------------------------------------------------------------------------------
*/
struct linux_dirent64 {
	ino64_t		d_ino;		// inode number
	loff_t		d_off;		// offset to next dirent
	unsigned short	d_reclen;	// length of this dirent
	unsigned char	d_type;		// file type
	char		d_name[];	// filename
					// length is actually (d_reclen - 2 -
					// offsetof(struct linux_dirent, d_name)
};

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
 Funtion	:init_fs
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize file system management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_fs(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_fs
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy fs management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_fs(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:make_init_fs
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:make initial file system space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int make_init_fs(void);

#endif	// __BK_FS_VFS_H__
