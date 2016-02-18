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

#ifndef	__BK_FS_FILE_SYSTEM_TYPE_H__
#define	__BK_FS_FILE_SYSTEM_TYPE_H__


/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct file_system_type;
struct super_block;
struct module;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
struct file_system_type {
	const char		*name;
	int			fs_flags;
	struct dentry* (*mount)(struct file_system_type *fs_type, int flags,
				const char *dev_name, void *data);
	void (*kill_sb)(struct super_block *sb);
	struct module		*owner;
	struct file_system_type	*next;
	struct list		fs_supers;
};

#define	FS_REQUIRES_DEV		0x00000001
#define	FS_BINARY_MOUNTDATA	0x00000002
#define	FS_HAS_SUBTYPE		0x00000004
#define	FS_USERNS_MOUNT		0x00000008
#define	FS_USERNS_DEV_MOUNT	0x00000010
#define	FS_USERNS_VISIBLE	0x00000020
#define	FS_RENAME_DOES_D_MOVE	0x00008000

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
 Funtion	:register_filesystem
 Input		:struct file_system_type *fs_type
 		 < file system type to register to the kernel >
 Output		:void
 Return		:int
 		 < result >
 Description	:register a file system type to the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int register_filesystem(struct file_system_type *fs_type);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_filesystem_type
 Input		:const char *name
 		 < name of file system to search >
 Output		:void
 Return		:struct file_system_type*
 		 < found file system type. return null if not found >
 Description	:search a list of file system type
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct file_system_type* get_filesystem_type(const char *name);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_filesystem
 Input		:struct file_sytem_type *fs_type
 		 < file system type to unregister from the system >
 Output		:void
 Return		:int
 		 < result >
 Description	:unregister a file system type from the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int unregister_filesystem(struct file_system_type *fs_type);

#endif	// __BK_FS_FILE_SYSTEM_TYPE_H__
