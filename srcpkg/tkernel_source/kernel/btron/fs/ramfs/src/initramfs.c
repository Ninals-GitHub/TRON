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

#include <string.h>

#include <libstr.h>
#include <tk/typedef.h>
#include <tstdlib/round.h>
#include <debug/vdebug.h>
#include <bk/fs/vfs.h>
#include <bk/fs/path.h>
#include <bk/fs/ramfs/ramfs.h>
#include <bk/memory/page.h>

#include <bk/uapi/sys/stat.h>

#include "cpio.h"

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct cpio_ope;

LOCAL int check_magic(union cpio_header *entry, struct cpio_ope **ope);
/*
----------------------------------------------------------------------------------
	old cpio header operations
----------------------------------------------------------------------------------
*/
LOCAL int old_cpio_create(union cpio_header *entry, struct cpio_ope *cpio_ope);
LOCAL char* old_get_filename(union cpio_header *entry);
LOCAL void* old_get_file(union cpio_header *entry);
LOCAL mode_t old_get_mode(union cpio_header *entry);
LOCAL dev_t old_get_rdev(union cpio_header *entry);
LOCAL size_t old_get_filesize(union cpio_header *entry);
int old_set_inode(union cpio_header *entry, struct vnode *vnode);
LOCAL union cpio_header* old_get_next(union cpio_header *entry);


/*
==================================================================================

	DEFINE 

==================================================================================
*/
struct cpio_ope {
	char* (*get_filename)(union cpio_header *entry);
	void* (*get_file)(union cpio_header *entry);
	mode_t (*get_mode)(union cpio_header *entry);
	dev_t (*get_rdev)(union cpio_header *entry);
	size_t (*get_filesize)(union cpio_header *entry);
	int (*set_inode)(union cpio_header *entry, struct vnode *vnode);
	union cpio_header *(*get_next)(union cpio_header *entry);
	int (*create)(union cpio_header *entry, struct cpio_ope *cpio_ope);
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct cpio_ope cpio_old_ope = {
	.get_filename	= old_get_filename,
	.get_file	= old_get_file,
	.get_mode	= old_get_mode,
	.get_rdev	= old_get_rdev,
	.get_filesize	= old_get_filesize,
	.set_inode	= old_set_inode,
	.get_next	= old_get_next,
	.create		= old_cpio_create,
};
LOCAL struct cpio_ope cpio_odc_ope = {
};
LOCAL struct cpio_ope cpio_newc_ope = {
};

LOCAL struct cpio_ope *cpio_ope;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:make_initramfs
 Input		:void *cpio
 		 < start address of initrd which is cpio format >
 Output		:void
 Return		:void
 Description	:make initramfs from initrd
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int make_initramfs(void *cpio)
{
	union cpio_header *header = (union cpio_header*)cpio;
	int ret;
	
	while (header) {
		mode_t mode;
		
		ret = check_magic(header, &cpio_ope);
		
		if (UNLIKELY(ret)) {
			break;
		}
		
		mode = cpio_ope->get_mode(header);
		
		/* ------------------------------------------------------------ */
		/* regular file							*/
		/* ------------------------------------------------------------ */
		if (S_ISREG(mode)) {
			//vd_printf("create a %s\n", cpio_ope->get_filename(header));
			ret = cpio_ope->create(header, cpio_ope);
		} else if (S_ISDIR(mode)) {
			//vd_printf("mkdir a %s\n", cpio_ope->get_filename(header));
			/* ---------------------------------------------------- */
			/* directory						*/
			/* ---------------------------------------------------- */
			//char *name = cpio_ope->get_filename(header);
			
			//ret = kmkdir(name, mode);
			ret = cpio_ope->create(header, cpio_ope);
		} else if (S_ISLNK(mode)) {
			char *linkpath;
			char *target;
			const char *f_target;
			size_t size = cpio_ope->get_filesize(header);
			
			/* ---------------------------------------------------- */
			/* symbolic link					*/
			/* ---------------------------------------------------- */
			linkpath = cpio_ope->get_filename(header);
			
			if (UNLIKELY(!linkpath)) {
				goto goto_next;
			}
			
			if (UNLIKELY(!size)) {
				goto goto_next;
			}
			
			f_target = (const char*)cpio_ope->get_file(header);
			
			if (UNLIKELY(f_target)) {
				goto goto_next;
			}
			
			target = (char*)kmalloc(size + 1, 0);
			
			if (UNLIKELY(!target)) {
				goto goto_next;
			}
			
			target = strncpy(target, f_target, size);
			
			vd_printf("symlink a target:%s linkpath:%s\n", target, linkpath);
			
			target[size] = '\0';
			
			ret = symlink(target, linkpath);
			
			kfree(target);
		} else {
			char *name = cpio_ope->get_filename(header);
			//vd_printf("mknod a %s\n", name);
			/* ---------------------------------------------------- */
			/* end marker						*/
			/* ---------------------------------------------------- */
			if (UNLIKELY(!strncmp(name,
					"TRAILER!!!", sizeof("TRAILER!!!")))) {
				break;
			}
			
			/* ---------------------------------------------------- */
			/* char or block device etc.				*/
			/* ---------------------------------------------------- */
			
			dev_t rdev = cpio_ope->get_rdev(header);
			
			ret = kmknod(name, mode, rdev);
		}
		
		/* ------------------------------------------------------------ */
		/* if a file already existed, trt next entry			*/
		/* ------------------------------------------------------------ */
		if (UNLIKELY(ret == -EEXIST)) {
			ret = 0;
		} else if (UNLIKELY(ret)) {
			panic("cpio error %d\n", ret);
			return(ret);
		}
		
goto_next:
		header = cpio_ope->get_next(header);
	}
	
	//vd_printf("cpio finished\n");
	
	show_dir_path("/bin");
#if 0
	{
		struct stat64_i386 buf;
		ret = lstat64("/bin/dmesg", &buf);
		
		printf("ret:%d\n", -ret);
	}
	for(;;);
#endif
	//show_dir_path("/etc");
	
	return(0);
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
 Funtion	:check_magic
 Input		:union cpio_header *entry
 		 < cpio entry buffer >
 		 struct cpio_ope **ope
 		 < cipo operations >
 Output		:struct cpio_ope **ope
 		 < cpio operation set by according to the magic >
 Return		:int
 		 < result >
 Description	:check magic
==================================================================================
*/
LOCAL int check_magic(union cpio_header *entry, struct cpio_ope **ope)
{
	int ret = -1;
	
	*ope = NULL;
	/* -------------------------------------------------------------------- */
	/* old binary format							*/
	/* -------------------------------------------------------------------- */
	if (entry->old.c_magic == CPIO_OLD_MAGIC) {
		*ope = &cpio_old_ope;
		ret = 0;
	} else if (strncmp(entry->odc.c_magic,
				CPIO_ODC_MAGIC, sizeof(CPIO_ODC_MAGIC) - 2) == 0) {
		/* ------------------------------------------------------------ */
		/* old ascii format						*/
		/* ------------------------------------------------------------ */
		if (entry->odc.c_magic[5] == '7') {
			*ope = &cpio_odc_ope;
			ret = 0;
		} else if (entry->newc.c_magic[5] == '1') {
			/* ---------------------------------------------------- */
			/* new ascii format					*/
			/* ---------------------------------------------------- */
			*ope = &cpio_newc_ope;
			ret = 0;
		}
	}
	
	return(ret);
}

/*
==================================================================================
 Funtion	:old_cpio_create
 Input		:union cpio_header *entry
 		 < cpio header entry >
 		 struct cpio_ope *cpio_ope
 		 < cpio operations >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a normal file
==================================================================================
*/
LOCAL int old_cpio_create(union cpio_header *entry, struct cpio_ope *cpio_ope)
{
	struct super_block *sb = get_root_sb(get_current());
	struct vnode *vnode;
	struct file_name *filename;
	void *file;
	size_t err;
	struct dentry *dentry;
	struct dentry *parent;
	
	err = vfs_lookup(cpio_ope->get_filename(entry), &filename, LOOKUP_CREATE);
	
	if (UNLIKELY(err)) {
		vd_printf("failed vfs_lookup[%s] at %s [%d]\n", cpio_ope->get_filename(entry), __func__, -1 * err);
		return(err);
	}
	
	parent = filename->parent;
	dentry = filename->dentry;
	
	put_file_name(filename);
	
	vnode = vfs_alloc_vnode(sb);
	
	if (!vnode) {
		return(-ENOMEM);
	}
	
	vnode->v_fops = &ramfs_file_ope;
	
	err = cpio_ope->set_inode(entry, vnode);
	
	file = cpio_ope->get_file(entry);
	
	dentry_associated(dentry, vnode);
	
	dentry_add_dir(parent, dentry);
	
	if (S_ISREG(vnode->v_mode)) {
		err = mem_page_cache(vnode, file, 0, 0, vnode->v_size);
		
		if (err == vnode->v_size) {
			err = 0;
		}
	}
	
	return(err);
}


/*
----------------------------------------------------------------------------------
	old cpio header operations
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:old_get_filename
 Input		:union cpio_header *entry
 		 < cpio header entry >
 Output		:void
 Return		:char *
 		 < file name >
 Description	:get a file name from the old cpio header
==================================================================================
*/
LOCAL char* old_get_filename(union cpio_header *entry)
{
	struct cpio_old_header *old = &entry->old;
	
	return((char*)(old + 1));
}

/*
==================================================================================
 Funtion	:old_get_file
 Input		:union cpio_header *entry
 		 < cpio header entry >
 Output		:void
 Return		:void *
 		 < file content >
 Description	:get file content from the old cpio header
==================================================================================
*/
LOCAL void* old_get_file(union cpio_header *entry)
{
	char *name = old_get_filename(entry);
	unsigned long namesize = entry->old.c_namesize;
	char *file;
	
	if (namesize & 0x1) {
		namesize++;
	}
	
	file = name + namesize;
	
	return((void*)file);
}

/*
==================================================================================
 Funtion	:old_get_mode
 Input		:union cpio_header *entry
 		 < cpio header entry >
 Output		:void
 Return		:mode_t
 		 < file permittion >
 Description	:get file permittion from the old cpio header
==================================================================================
*/
LOCAL mode_t old_get_mode(union cpio_header *entry)
{
	return((mode_t)entry->old.c_mode);
}

/*
==================================================================================
 Funtion	:old_get_rdev
 Input		:union cpio_header *entry
 		 < cpio header entry >
 Output		:void
 Return		:dev_t
 		 < device number >
 Description	:get device number from the old cpio header
==================================================================================
*/
LOCAL dev_t old_get_rdev(union cpio_header *entry)
{
	return((dev_t)entry->old.c_rdev);
}

/*
==================================================================================
 Funtion	:old_get_filesize
 Input		:union cpio_header *entry
 		 < cpio header entry >
 Output		:void
 Return		:size_t
 		 < file size >
 Description	:get file size of the old cpio header entry
==================================================================================
*/
LOCAL size_t old_get_filesize(union cpio_header *entry)
{
	struct cpio_old_header *old = &entry->old;
	size_t file_size;
	
	file_size = ((size_t)old->c_filesize[0] << 16)
					+ (size_t)old->c_filesize[1];
	
	return(file_size);
}


/*
==================================================================================
 Funtion	:old_set_inode
 Input		:union cpio_header *entry
 		 < cpio header entry >
 		 struct vnode *vnode
 		 < vnode to set up >
 Output		:struct vnode *vnode
 		 < initialized >
 Return		:int
 		 < result >
 Description	:set up a vnode information from a cpio header
==================================================================================
*/
int old_set_inode(union cpio_header *entry, struct vnode *vnode)
{
	struct cpio_old_header *old = &entry->old;

	vnode->v_mode	= (umode_t)old->c_mode;
	if (S_ISDIR(vnode->v_mode)) {
		vnode->v_size = PAGESIZE;
	} else {
		vnode->v_size	= ((loff_t)old->c_filesize[0] << 16) +
					(loff_t)old->c_filesize[1];
	}
	vnode->v_blocks	= ROUNDUP(vnode->v_size, S_BLKSIZE);
	vnode->v_uid	= (uid32_t)old->c_uid;
	vnode->v_gid	= (gid32_t)old->c_gid;
	vnode->v_nlink	= (unsigned int)old->c_nlink;
	
	vnode->v_mtime.tv_sec = ((time_t)old->c_mtime[0] << 16) +
				(time_t)old->c_mtime[1];
	vnode->v_ctime.tv_sec = vnode->v_atime.tv_sec = vnode->v_mtime.tv_sec;
	
	if (S_ISBLK(vnode->v_mode) || S_ISCHR(vnode->v_mode)) {
		vnode->v_rdev = (dev_t)old->c_rdev;
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:old_get_next
 Input		:union cpio_header *entry
 		 < cpio header entry >
 Output		:void
 Return		:union cpio_header *
 		 < next entry >
 Description	:get next cpio header entry
==================================================================================
*/
LOCAL union cpio_header* old_get_next(union cpio_header *entry)
{
	struct cpio_old_header *old = &entry->old;
	char *next = old_get_file(entry);
	unsigned long filesize;
	
	filesize = (old->c_filesize[0] << 16) + old->c_filesize[1];
	
	if (filesize & 0x1) {
		filesize++;
	}
	
	next += filesize;
	
	return((union cpio_header*)next);
}

/*
----------------------------------------------------------------------------------
	old cpio ascii header operations
----------------------------------------------------------------------------------
*/

/*
----------------------------------------------------------------------------------
	new cpio ascii header operations
----------------------------------------------------------------------------------
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
