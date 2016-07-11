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
#include <bk/memory/vm.h>
#include <bk/uapi/sys/stat.h>

#include <libstr.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL struct dentry*
dentry_cache_alloc(struct super_block *sb, const struct qstr *name);
LOCAL void dentry_cache_free(struct dentry *dentry);
LOCAL int compare_dentry_name(const struct qstr *name, struct dentry *dentry);


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
LOCAL struct kmem_cache *dentry_cache;
LOCAL const char dentry_cache_name[] = "dentry_cache";

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_dentry
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize dentry management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_dentry(void)
{
	dentry_cache = kmem_cache_create(dentry_cache_name,
					sizeof(struct dentry), 0, 0, NULL);
	
	if (UNLIKELY(!dentry_cache)) {
		vd_printf("error:dentry_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_dentry_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a dentry cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_dentry_cache(void)
{
	kmem_cache_destroy(dentry_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_alloc_dentry
 Input		:struct vnode *vnode
 		 < vnode to be associated with a new dentry >
 		 struct dentry *parent
 		 < a parent of a new dentry >
 		 struct qstr *name
 		 < name for a new dentry >
 Output		:void
 Return		:struct dentry*
 		 < an allocated dentry >
 Description	:allocate a new dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct dentry*
vfs_alloc_dentry(struct vnode *vnode, struct dentry *parent, struct qstr *name)
{
	struct dentry *dentry;
	
	if (vnode) {
		dentry= dentry_cache_alloc(vnode->v_sb, name);
	} else {
		// vnode is negative
		dentry= dentry_cache_alloc(parent->d_sb, name);
	}
	
	if (UNLIKELY(!dentry)) {
		return(NULL);
	}
	
	atomic_inc(&dentry->d_count);
	
	if (UNLIKELY(!parent)) {
		dentry->d_parent = dentry;
	} else {
		dentry->d_parent = parent;
	}
	
	if (vnode) {
		dentry->d_sb = vnode->v_sb;
		add_list(&dentry->d_alias, &vnode->v_dentry);
	} else {
		dentry->d_sb = parent->d_sb;
	}
	dentry->d_vnode = vnode;
	
	//if (LIKELY(parent)) {
	//	add_list(&parent->d_subdirs, &dentry->d_child);
	//}
	
	return(dentry);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_root_dentry
 Input		:struct vnode *root
 		 < root vnode >
 Output		:void
 Return		:struct dentry*
 		 < root dentry >
 Description	:allocate a root dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct dentry* alloc_root_dentry(struct vnode *root)
{
	struct dentry *root_dentry;
	const struct qstr name_root = {
		.name = "/",
		.len = 1,
	};
	
	if (UNLIKELY(!root)) {
		return(NULL);
	}
	
	root_dentry = vfs_alloc_dentry(root, NULL, (struct qstr*)&name_root);
	
	return(root_dentry);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_free_dentry
 Input		:struct dentry *dentry
 		 < directory entry to free >
 Output		:void
 Return		:void
 Description	:free a directory entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vfs_free_dentry(struct dentry *dentry)
{
	if (!atomic_dec_and_test(&dentry->d_count)) {
		dentry_cache_free(dentry);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lookup_dentry_children
 Input		:struct dentry *dir
 		 < dentry of directory to lookup in >
 		 const struct qstr *name
 		 < name to lookup >
 		 unsigned int flags
 		 < look up flags >
 Output		:void
 Return		:struct dentry*
 		 < found result >
 Description	:lookup name in a dentry of directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct dentry*
lookup_dentry_children(struct dentry *dir, const struct qstr *name,
						unsigned int flags)
{
	struct dentry *dentry;
	struct dentry *temp;
	int result;
	
	if (UNLIKELY(is_empty_list(&dir->d_subdirs))) {
		return(NULL);
	}
	
	if (UNLIKELY(is_lookup_test(flags))) {
		show_subdirs(dir);
	}
	
	list_for_each_entry_safe(dentry, temp, &dir->d_subdirs, d_child) {
		result = compare_dentry_name(name, dentry);
#if 0
		if (UNLIKELY(is_lookup_test(flags))) {
			char *d_name;
			if (dentry->d_name.name) {
				d_name = dentry->d_name.name;
			} else {
				d_name = dentry->d_iname;
			}
			vd_printf("name:%s dentry:%s result:%d\n", name->name, d_name, result);
		}
#endif
		if (result == 0) {
			/* found						*/
			return(dentry);
		}
	}
	
	return(NULL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_associated
 Input		:struct dentry *dentry
 		 < dentry to associate with its vnode >
 		 struct vnode *vnode
 		 < vnode to be associted with >
 Output		:void
 Return		:void
 Description	:associate dentry with its vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void dentry_associated(struct dentry *dentry, struct vnode *vnode)
{
	dentry->d_vnode = vnode;
	add_list(&dentry->d_alias, &vnode->v_dentry);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_add_dir
 Input		:struct dentry *dir
 		 < parent directory to which add the new entry >
 		 struct dentry *dentry
 		 < dentry to add to its parent directory >
 Output		:void
 Return		:void
 Description	:add dentry to its parent directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void dentry_add_dir(struct dentry *dir, struct dentry *dentry)
{
	add_list_tail(&dentry->d_child, &dir->d_subdirs);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_subdirs
 Input		:struct dentry *dir
 		 < directory to show its children >
 Output		:void
 Return		:void
 Description	:show subdirectories
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void show_subdirs(struct dentry *dir)
{
	struct dentry *dentry;
	
	vd_printf("dir[%s] list: ", dentry_name(dir));
	
	list_for_each_entry(dentry, &dir->d_subdirs, d_child) {
		vd_printf("%s ", dentry_name(dentry));
	}
	vd_printf("\n");
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
 Funtion	:dentry_cache_alloc
 Input		:struct super_block *sb
 		 < suber block to which a new dentry belongs >
 		 const struct qstr *name
 		 < name of a new entry >
 Output		:void
 Return		:struct dentry*
 		 < dentry object >
 Description	:allocate a dentry object
==================================================================================
*/
LOCAL struct dentry*
dentry_cache_alloc(struct super_block *sb, const struct qstr *name)
{
	char *d_name;
	struct dentry *dentry = (struct dentry*)kmem_cache_alloc(dentry_cache, 0);
	
	if (UNLIKELY(!dentry)) {
		return(dentry);
	}
	
	memset((void*)dentry, 0x00, sizeof(struct dentry));
	
	init_list(&dentry->d_lru);
	init_list(&dentry->d_child);
	init_list(&dentry->d_subdirs);
	init_list(&dentry->d_alias);
	
	if ((DENTRY_NAME_LEN - 1) < name->len) {
		dentry->d_name.name = kmalloc(name->len + 1, 0);
		if (!dentry->d_name.name) {
			dentry_cache_free(dentry);
			return(NULL);
		}
		memcpy((void*)dentry->d_name.name,
			(void*)name->name, name->len);
		d_name = (char*)dentry->d_name.name;
		
	} else {
		memcpy((void*)dentry->d_iname, (void*)name->name, name->len);
		d_name = (char*)dentry->d_iname;
	}
	
	d_name[name->len] = '\0';
	
	dentry->d_name.len = name->len;
	
	dentry->d_sb = sb;
	dentry->d_op = (struct dentry_operations*)sb->s_d_op;
	
	return(dentry);
}

/*
==================================================================================
 Funtion	:dentry_cache_free
 Input		:struct dentry *dentry
 		 < dentry to free object>
 Output		:void
 Return		:void
 Description	:free a dentry object
==================================================================================
*/
LOCAL void dentry_cache_free(struct dentry *dentry)
{
	kmem_cache_free(dentry_cache, dentry);
}

/*
==================================================================================
 Funtion	:compare_dentry_name
 Input		:struct 
 		 const struct qstr *name
 		 < name to compare with >
 		 struct dentry *dentry
 		 < dentry to compare its name >
 Output		:void
 Return		:int
 		 < result >
 Description	:compare name with dentry's name
==================================================================================
*/
LOCAL int compare_dentry_name(const struct qstr *name, struct dentry *dentry)
{
	int result;
	int len;
	char *cmp_name;
	
	if (dentry->d_name.name) {
		cmp_name = dentry->d_name.name;
	} else {
		cmp_name = dentry->d_iname;
	}
	
	if (dentry->d_name.len < name->len) {
		len = name->len;
	} else {
		len = dentry->d_name.len;
	}
	
	if (dentry->d_op && dentry->d_op->d_compare) {
		result = dentry->d_op->d_compare(dentry->d_parent,
							dentry,
							len,
							cmp_name,
							name);
		return(result);
	}
	
	return(strncmp(name->name, cmp_name, len));
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
