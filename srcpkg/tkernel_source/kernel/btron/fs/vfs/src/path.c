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
 Funtion	:dup_pathname
 Input		:const char *pathname
 		 < path name to duplicate >
 Output		:void
 Return		:struct qstr*
 		 < alocated and duplicated string >
 Description	:duplicate a pathname
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct qstr* dup_pathname(const char *pathname)
{
	struct qstr *new_path;
	
	new_path = kmalloc(sizeof(struct qstr), 0);
	
	if (UNLIKELY(!new_path)) {
		return(NULL);
	}
	
	new_path->name = kmalloc(PATH_MAX, 0);
	
	if (UNLIKELY(!new_path->name)) {
		goto error_out;
	}
	
#if 0
	new_path->len = PATH_MAX;
	
	new_path->len = strncpy(new_path->name, pathname, PATH_MAX);
	
	if (UNLIKELY(!new_path->len)) {
		vd_printf("dup_pathname:cannot copy[%s]\n", pathname);
		goto error_out;
	}
	
	if (UNLIKELY(new_path->len < 0)) {
		vd_printf("dup_pathname:user memory access violation[%d]\n", new_path->len);
		goto error_out;
	}
#endif
	new_path->len = kstrncpy_len(new_path->name, pathname, PATH_MAX, 0);
	
	if (PATH_MAX <= new_path->len) {
		vd_printf("1:unexpected error %s\n", __func__);
		goto name_too_long;
	}
	
	return(new_path);
	
error_out:
	put_pathname(new_path);
	return(NULL);
	
name_too_long:
	put_pathname(new_path);
	return(PTR_ERR_NAME_TOO_LONG);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:put_pathname
 Input		:struct qstr *pathname
 		 < pathname to put >
 Output		:void
 Return		:void
 Description	:put a path name memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void put_pathname(struct qstr *pathname)
{
	if (pathname && pathname->name) {
		kfree(pathname->name);
	}
	
	if (pathname) {
		kfree(pathname);
	}
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:parse_path
 Input		:cnost struct *qstr
 		 < path name to parse >
 		 struct path *dir_path
 		 < directory path to start to look up >
 		 unsigned int flags
 		 < lookup flags >
 Output		:void
 Return		:struct file_name*
 Description	:parse the path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct file_name*
parse_path(const struct qstr *path, struct path *dir_path, unsigned int flags)
{
	struct dentry *dentry;
	struct dentry *dir = dir_path->dentry;
	struct dentry *found;
	char *path_name = (char*)path->name;
	char buf[NAME_MAX];
	struct qstr entry_name;
	struct file_name *fname;
	
	fname = (struct file_name*)kcalloc(1, sizeof(struct file_name), 0);
	
	if (UNLIKELY(!fname)) {
		return(NULL);
	}
	
	entry_name.name = buf;
	
	fname->mnt = dir_path->mnt;
	
next_parse:
	/* -------------------------------------------------------------------- */
	/* remove '/'								*/
	/* -------------------------------------------------------------------- */
	while (*path_name && *path_name == '/') {
		path_name++;
	}
	
	if (UNLIKELY(*path_name == '\0')) {
		fname->dentry = dir;
		if(dir->d_vnode && S_ISDIR(dir->d_vnode->v_mode)) {
			fname->parent = dir;
		} else {
			fname->parent = dir->d_parent;
		}
		fname->filename = dup_pathname(entry_name.name);
		if (UNLIKELY(is_lookup_test(flags))) {
			vd_printf("found:%s\n", fname->filename->name);
		}
		return(fname);
	}
	
	entry_name.len = 0;
	
	while (*path_name && *path_name != '/') {
		entry_name.name[entry_name.len++] = *path_name++;
	}
	
	entry_name.name[entry_name.len] = '\0';
	
	if (UNLIKELY(is_lookup_test(flags))) {
		vd_printf("parse_path:%s ", entry_name.name);
		if (dir->d_name.name) {
			vd_printf("dir name1:%s\n", dir->d_name.name);
		} else {
			vd_printf("dir name2:%s\n", dir->d_iname);
		}
	}
	
	if (UNLIKELY(entry_name.len == 1)) {
		if (entry_name.name[0] == '.') {
			goto next_parse;
		}
	} else if (UNLIKELY(entry_name.len == 2)) {
		if (entry_name.name[0] == '.' && entry_name.name[1] == '.') {
			dir = dir->d_parent;
			goto next_parse;
		}
	}
	
	/* -------------------------------------------------------------------- */
	/* lookup dir								*/
	/* -------------------------------------------------------------------- */
	dentry = lookup_dentry_children(dir, (const struct qstr*)&entry_name,
						flags);
	if (UNLIKELY(is_lookup_test(flags))) {
		if (dentry) {
			char *dentry_name;
			if (dentry->d_name.name) {
				dentry_name = dentry->d_name.name;
			} else {
				dentry_name = dentry->d_iname;
			}
			vd_printf("dentry:%s\n", dentry_name);
			
			if (!dentry->d_vnode) {
				vd_printf("unexpected negative dentry!!!!!\n");
			}
		}
	}
	
	if (!dentry) {
		if (UNLIKELY(is_lookup_create(flags))) {
			while (*path_name && *path_name == '/') {
				path_name++;
			}
			
			if (*path_name == '\0') {
				goto not_found_create;
			}
		}
		if (UNLIKELY(is_lookup_test(flags))) {
			vd_printf("not_found:%s\n", entry_name.name);
		}
		goto not_found;
	} else if (dentry && dentry->d_vnode) {
		mode_t mode = dentry->d_vnode->v_mode;
		
#if 0	// caused gfp, must be fixed later
		if (UNLIKELY(S_ISLNK(mode)) && is_lookup_followlink(flags)) {
			int i = 0;
			int err;
			struct file_name *sym_found;
			struct vnode *symnode;
			char *symname;
			
			if (UNLIKELY(is_lookup_test(flags))) {
				vd_printf("follow link:%s\n", entry_name.name);
			}
			
			/* -----------------------------------------------------*/
			/* remove '/'						*/
			/* ---------------------------------------------------- */
			while (*path_name && *path_name == '/') {
				path_name++;
			}
			/* -----------------------------------------------------*/
			/* save current path					*/
			/* ---------------------------------------------------- */
			while (*path_name != '\0') {
				path->name[i++] = *path_name++;
			}
			
			path->name[i] = '\0';
			
			symnode = fname->dentry->d_vnode;
			
			if (symnode->v_op && symnode->v_op->readlink) {
				symname = (char*)kmalloc(PATH_MAX, 0);
				
				if (UNLIKELY(!symname)) {
					goto no_memory;
				}
				err = symnode->v_op->readlink(fname->dentry,
							symname, PATH_MAX);
				
				if (UNLIKELY(err)) {
					kfree(symname);
					goto error_out;
				}
			} else {
				symname = kstrndup(symnode->v_link, PATH_MAX, 0 );
				
				if (UNLIKELY(!symname)) {
					goto no_memory;
				}
			}
			
			err = follow_link(symname, &sym_found, flags);
			
			if (UNLIKELY(err)) {
				kfree(symname);
				goto error_out;
			}
			
			dir = sym_found->dentry;
			
			put_file_name(sym_found);
			
			return(fname);
		} else {
			/* found, go to next path element			*/
			dir = dentry;
		}
#else
		dir = dentry;
#endif
		
		goto next_parse;
	}
	
	if (dir->d_vnode->v_op && dir->d_vnode->v_op->lookup) {
		found = dir->d_vnode->v_op->lookup(dir->d_vnode, dentry, 0);
		
		if (!found) {
			if (UNLIKELY(is_lookup_create(flags))) {
				goto not_found_create;
			} else {
				goto not_found;
			}
		}
		
		if (!found->d_vnode) {
			/* found negative dentry				*/
			put_file_name(fname);
			return(NULL);
		}
		
		dir = found;
		goto next_parse;
	}
	
	if (UNLIKELY(is_lookup_test(flags))) {
		vd_printf("alloc negative dentry:%s\n", entry_name.name);
	}
	
	dentry = vfs_alloc_negative_dentry(dir, &entry_name);
	
	dentry_add_dir(dir, dentry);
	
	if (UNLIKELY(!dentry)) {
		put_file_name(fname);
		return(NULL);
	}
	
	return(NULL);
	
not_found_create:
	if (UNLIKELY(is_lookup_test(flags))) {
		vd_printf("alloc negative dentry with creation:%s\n", entry_name.name);
	}
	dentry = vfs_alloc_negative_dentry(dir, &entry_name);
	fname->parent = dir;
	fname->dentry = dentry;
	if (UNLIKELY(is_lookup_test(flags))) {
		vd_printf("dir:%s dentry:%s\n", dir->d_iname, dentry->d_iname);
	}
	return(fname);
	
error_out:
no_memory:
not_found:
	put_file_name(fname);
	
	/* not found, return negative dentry					*/
	return(NULL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_file_name
 Input		:const struct qstr *filename
 		 < file name >
 Output		:void
 Return		:struct file_name*
 		 < allocated memory >
 Description	:allocate file name structure
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct file_name* alloc_file_name(const struct qstr *filename)
{
	struct file_name *fname;
	
	fname = kcalloc(1, sizeof(struct  file_name), 0);
	
	if (UNLIKELY(!fname)) {
		return(NULL);
	}
	
	fname->filename = dup_pathname(filename->name);
	
	if (UNLIKELY(!fname->filename)) {
		put_file_name(fname);
		
		return(NULL);
	}
	
	return(fname);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:put_file_name
 Input		:struct file_name *fname
 		 < file name structure to free >
 Output		:void
 Return		:void
 Description	:free a memory of file name structure
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void put_file_name(struct file_name *fname)
{
	if (UNLIKELY(!fname)) {
		return;
	}
	
	if (fname->filename) {
		put_pathname(fname->filename);
	}
	
	kfree(fname);
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
