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

#ifndef	__BK_FS_RAMFS_CPIO_H__
#define	__BK_FS_RAMFS_CPIO_H__

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
#define	CPIO_OLD_MAGIC		0070707
#define	CPIO_ODC_MAGIC		"070707"
#define	CPIO_NEW_MAGIC		"070701"


struct cpio_old_header {
	unsigned short	c_magic;
	unsigned short	c_dev;
	unsigned short	c_ino;
	unsigned short	c_mode;
	unsigned short	c_uid;
	unsigned short	c_gid;
	unsigned short	c_nlink;
	unsigned short	c_rdev;
	unsigned short	c_mtime[2];
	unsigned short	c_namesize;
	unsigned short	c_filesize[2];
};

struct cpio_odc_header {
	char		c_magic[6];
	char		c_dev[6];
	char		c_ino[6];
	char		c_mode[6];
	char		c_uid[6];
	char		c_gid[6];
	char		c_nlink[6];
	char		c_rdev[6];
	char		c_mtime[11];
	char		c_namesize[6];
	char		c_filesize[11];
};

struct cpio_newc_header {
	char		c_magic[6];
	char		c_ino[8];
	char		c_mode[8];
	char		c_uid[8];
	char		c_gid[8];
	char		c_nlink[8];
	char		c_mtime[8];
	char		c_filesize[8];
	char		c_devmajor[8];
	char		c_devminor[8];
};

union cpio_header {
	struct cpio_old_header	old;
	struct cpio_odc_header	odc;
	struct cpio_newc_header	newc;
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_FS_RAMFS_CPIO_H__
