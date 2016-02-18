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

#include <bk/kernel.h>
#include <bk/fs/vfs.h>
#include <bk/memory/slab.h>
#include <bk/uapi/sys/stat.h>

#include <t2ex/limits.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int
setup_vnode(struct vnode *dir, struct dentry *dentry, struct vnode *vnode);
LOCAL struct file_name* lookup(const struct qstr *path_name, unsigned int flags);

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
LOCAL struct kmem_cache *vnode_cache;
LOCAL const char vnode_cache_name[] = "vnode_cache";

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_vnode
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize vnode management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_vnode(void)
{
	vnode_cache = kmem_cache_create(vnode_cache_name,
					sizeof(struct vnode), 0, 0, NULL);
	
	if (UNLIKELY(!vnode_cache)) {
		vd_printf("error:vnode_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_vnode_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a vnode cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_vnode_cache(void)
{
	kmem_cache_destroy(vnode_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vnode_cache_alloc
 Input		:void
 Output		:void
 Return		:struct vnode*
 		 < vnode object >
 Description	:allocate a vnode object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vnode* vnode_cache_alloc(void)
{
	struct vnode *vnode = (struct vnode*)kmem_cache_alloc(vnode_cache, 0);
	
	if (UNLIKELY(!vnode)) {
		return(vnode);
	}
	
	INIT_VNODE(vnode);
	
	return(vnode);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vnode_cache_free
 Input		:struct vnode *vnode
 		 < vnode to free >
 Output		:void
 Return		:void
 Description	:free a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vnode_cache_free(struct vnode *vnode)
{
	kmem_cache_free(vnode_cache, vnode);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:INIT_VNODE
 Input		:struct vnode *vnode
 		 < vnode object to initialize >
 Output		:void
 Return		:void
 Description	:initialize vnode object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void INIT_VNODE(struct vnode *vnode)
{
	memset((void*)vnode, 0x00, sizeof(struct vnode));
	
	init_list(&vnode->v_dentry);
	init_list(&vnode->v_devices);
	init_list(&vnode->v_sb_list);
	init_list(&vnode->v_lru);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_special_vnode
 Input		:struct vnode *vnode
 		 < special vnode >
 		 umode_t mode
 		 < mode >
 		 dev_t dev
 		 < device number >
 Output		:void
 Return		:void
 Description	:initialize a special vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void init_special_vnode(struct vnode *vnode, umode_t mode, dev_t dev)
{
	vnode->v_mode = mode;
	
	if (S_ISCHR(mode)) {
		vnode->v_rdev = dev;
	} else if (S_ISBLK(mode)) {
		vnode->v_rdev = dev;
	} else if (S_ISFIFO(mode)) {
	}
}

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mknod
 Input		:const char *pathname
 		 < pathname to create a special file >
 		 mode_t mode
 		 < file creation mode >
 		 dev_t dev
 		 < device number of a special file >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a special file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int mknod(const char *pathname, mode_t mode, dev_t dev)
{
	struct file_name *filename;
	struct dentry *dentry;
	struct vnode *dir;
	int err;
	
	err = vfs_lookup(pathname, &filename, LOOKUP_CREATE);
	
	if (err) {
		return(err);
	}
	
	dir = filename->parent->d_vnode;
	dentry = filename->dentry;
	
	dentry_add_dir(filename->parent, filename->dentry);
	
	put_file_name(filename);
	
	return(vfs_mknod(dir, dentry, mode, dev));
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mkdir
 Input		:const char *pathname
 		 < path name for new directory >
 		 mode_t mode
 		 < file creation mode >
 Output		:void
 Return		:int
 		 < result >
 Description	:make a new directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int mkdir(const char *pathname, mode_t mode)
{
	struct file_name *fname;
	struct dentry *dentry;
	struct vnode *dir;
	int err;
	
	err = vfs_lookup(pathname, &fname, LOOKUP_CREATE);
	
	if (err) {
		return(err);
	}
	
	vd_printf("mkdir:%s\n", pathname);
	
	dir = fname->parent->d_vnode;
	dentry = fname->dentry;
	
	dentry_add_dir(fname->parent, fname->dentry);
	
	put_file_name(fname);
	
	return(vfs_mkdir(dir, dentry, mode));
}


/*
----------------------------------------------------------------------------------
	vfs vnode operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_lookup
 Input		:const char *pathname
 		 < path name to look up >
 		 struct file_name **found
 		 < looked up dentry result >
 		 unsigned int flags
 		 < lookup flags >
 Output		:struct file_name **found
 		 < looked up dentry result >
 Return		:int
 		 < result >
 Description	:lookup dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_lookup(const char *pathname, struct file_name **found, unsigned int flags)
{
	struct qstr *kpathname;
	int err;
	
	if (UNLIKELY(!pathname)) {
		err = -EFAULT;
		goto error_out;
	}
	
	kpathname = dup_pathname(pathname);
	
	if (UNLIKELY(!kpathname)) {
		vd_printf("failed dup pathname\n");
		err = -EFAULT;
		goto error_out;
	}
	
	if (UNLIKELY(kpathname == PTR_ERR_NAME_TOO_LONG)) {
		vd_printf("vfs_lookup:name is too long\n");
		err = -ENAMETOOLONG;
		goto error_out;
	}
	
	*found = lookup(kpathname, flags);
	
	put_pathname(kpathname);
	
	if (!*found) {
		vd_printf("vfs_lookup:cannot found %s\n", pathname);
		err = -ENOENT;
		goto error_out;
	}
	
	if (UNLIKELY(flags & LOOKUP_CREATE)) {
		if (!(*found)->dentry->d_vnode) {
			return(0);
		}
		vd_printf("vfs_lookup:LOOKUP_CREATE failed\n");
		put_file_name(*found);
		err = -EEXIST;
		goto error_out;
	}
	
	return(0);
	
error_out:
	*found = NULL;
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_mknod
 Input		:struct vnode *dir
 		 < a directory in which a special file is made >
 		 struct dentry *dentry
 		 < directory entry for a special file >
 		 umode_t mode
 		 < file open mode >
 		 dev_t dev
 		 < device number >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs make a special file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_mknod(struct vnode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct vnode *vnode;
	
	if (dir->v_op && dir->v_op->mknod) {
		return(dir->v_op->mknod(dir, dentry, mode, dev));
	}
	
	if (UNLIKELY(S_ISREG(mode) || S_ISDIR(mode))) {
		return(-ENXIO);
	}
	
	vnode = vnode_cache_alloc();
	
	if (UNLIKELY(!vnode)) {
		return(-ENOMEM);
	}
	
	init_special_vnode(vnode, mode, dev);
	
	setup_vnode(dir, dentry, vnode);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_create
 Input		:struct vnode *dir
 		 < a directory in which a file is made >
 		 struct dentry *dentry
 		 < directory entry for a file >
 		 umode_t mode
 		 < file open mode >
 		 bool excl
 		 < boolean : exclude create a file from other threads >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs create a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_create(struct vnode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	struct vnode *vnode;
	int err;
	
	if (dir->v_op && dir->v_op->create) {
		return(dir->v_op->create(dir, dentry, mode, excl));
	}
	
	if (UNLIKELY(!S_ISREG(mode))) {
		return(-ENXIO);
	}
	
	vnode = vnode_cache_alloc();
	
	if (UNLIKELY(!vnode)) {
		return(-ENOMEM);
	}
	
	vnode->v_mode = S_IFREG | mode;
	
	err = setup_vnode(dir, dentry, vnode);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_mkdir
 Input		:struct vnode *dir
 		 < a directory in which a new directory is made >
 		 struct dentry *dentry
 		 < directory entry for a new directory >
 		 umode_t mode
 		 < directory creation mode >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a new directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vfs_mkdir(struct vnode *dir, struct dentry *dentry, umode_t mode)
{
	struct vnode *vnode;
	
	if (dir->v_op && dir->v_op->mkdir) {
		return(dir->v_op->mkdir(dir, dentry, mode));
	}
	
	vnode = vnode_cache_alloc();
	
	if (UNLIKELY(!vnode)) {
		return(-ENOMEM);
	}
	
	vnode->v_mode = S_IFDIR | mode;
	
	vd_printf("vfs_mkdir:");
	
	if (dentry->d_name.name) {
		vd_printf("dentry name:%s\n", dentry->d_name.name);
	} else {
		vd_printf("dentry name:%s\n", dentry->d_iname);
	}
	
	
	return(setup_vnode(dir, dentry, vnode));
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
 Funtion	:setup_vnode
 Input		:struct vnode *dir
 		 < directory vnode >
 		 struct dentry *dentry
 		 < dentry of a file >
 		 struct vnode *vnode
 		 < vnode to set up >
 Output		:void
 Return		:int
 		 < result >
 Description	:set up a vnode. before calling this function, must be set v_mode
==================================================================================
*/
LOCAL int
setup_vnode(struct vnode *dir, struct dentry *dentry, struct vnode *vnode)
{
	/* -------------------------------------------------------------------- */
	/* file attributes							*/
	/* -------------------------------------------------------------------- */
	vnode->v_uid = dir->v_uid;
	vnode->v_gid = dir->v_gid;
	vnode->v_nlink++;
	atomic_inc(&vnode->v_count);
	
	/* -------------------------------------------------------------------- */
	/* set up vnode								*/
	/* -------------------------------------------------------------------- */
	vnode->v_op = dir->v_op;
	vnode->v_fops = dir->v_fops;
	
	/* -------------------------------------------------------------------- */
	/* set up dentry relationship						*/
	/* -------------------------------------------------------------------- */
	dentry_associated(dentry, vnode);
	add_list(&dentry->d_parent->d_subdirs, &dentry->d_child);
	
	return(0);
}

/*
==================================================================================
 Funtion	:lookup
 Input		:const struct qstr *path_name
 		 < path to lookup >
 		 unsigned int flags
 		 < lookup flags >
 Output		:void
 Return		:struct file_name*
 		 < looked up >
 Description	:look up an entry in a directory
==================================================================================
*/
LOCAL struct file_name* lookup(const struct qstr *path_name, unsigned int flags)
{
	struct path *path;
	struct file_name *fname;
	struct dentry *parent;
	struct dentry *dentry;
	
	if (UNLIKELY(!path_name->len)) {
		return(NULL);
	}
	
	if (UNLIKELY(!path_name->name)) {
		return(NULL);
	}
	
	/* -------------------------------------------------------------------- */
	/* absolute path : path name starts '/'					*/
	/* -------------------------------------------------------------------- */
	if (path_name->name[0] == '/') {
		path = get_root(get_current());
		
		if (UNLIKELY(path_name->len == 1)) {
			parent = path->dentry;
			dentry = path->dentry;

		}
	/* -------------------------------------------------------------------- */
	/* relative path : path name starts except for '/'			*/
	/* -------------------------------------------------------------------- */
	} else {
		path = get_cwd(get_current());
		
		if (path_name->len == 1 && path_name->name[0] == '.') {
			parent = path->dentry->d_parent;
			dentry = path->dentry;
			goto found_dots;
		} else if (path_name->len == 2 &&
				path_name->name[0] == '.' &&
				path_name->name[1] == '.') {
			if (LIKELY(path->dentry->d_parent)) {
				parent = path->dentry->d_parent;
				dentry = parent;
				goto found_dots;
			}
			
			path = get_root(get_current());
			
			parent = path->dentry;
			dentry = path->dentry;
			
			goto found_dots;
		}
	}
	
	/* -------------------------------------------------------------------- */
	/* parse a path								*/
	/* -------------------------------------------------------------------- */
	return(parse_path(path_name, path->dentry, flags));

found_dots:
	fname = alloc_file_name(path_name);
	
	if (UNLIKELY(!fname)) {
		return(NULL);
	}
	
	fname->parent = parent;
	fname->dentry = dentry;
	
	return(fname);

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
