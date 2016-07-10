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

#ifndef	__BK_FS_PATH_H__
#define	__BK_FS_PATH_H__

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct vfsmount;
struct dentry;
struct qstr;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
struct path {
	struct vfsmount	*mnt;
	struct dentry	*dentry;
};

#define	PTR_ERR_NAME_TOO_LONG	((struct qstr*)(-2))

/*
----------------------------------------------------------------------------------
	lookup file name
----------------------------------------------------------------------------------
*/
struct file_name {
	struct dentry	*parent;
	struct dentry	*dentry;
	struct vfsmount	*mnt;
	struct qstr	*filename;
	int		follow_count;
};

/*
----------------------------------------------------------------------------------
	lookup flags
----------------------------------------------------------------------------------
*/
#define	LOOKUP_ENTRY		0x00000000
#define	LOOKUP_CREATE		0x00000001
#define	LOOKUP_FOLLOW_LINK	0x00000002

#define	LOOKUP_TEST		0x80000000

/*
==================================================================================

	Management 

==================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_path
 Input		:struct path *to
 		 < copy to >
 		 struct path *from
 		 < copy from >
 Output		:struct path *to
 		 < copy to >
 Return		:void
 Description	:copy path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE void copy_path(struct path *to, struct path *from)
{
	to->mnt = from->mnt;
	to->dentry = from->dentry;
}

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
IMPORT struct qstr* dup_pathname(const char *pathname);

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
IMPORT void put_pathname(struct qstr *pathname);

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
IMPORT struct file_name*
parse_path(const struct qstr *path, struct path *dir_path, unsigned int flags);


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
IMPORT struct file_name* alloc_file_name(const struct qstr *filename);

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
IMPORT void put_file_name(struct file_name *fname);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_lookup_create
 Input		:unsigned int flags
 		 < look up flags >
 Output		:void
 Return		:int
 		 < bool result >
 Description	:is a create flag set
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE int is_lookup_create(unsigned int flags)
{
	return(flags & LOOKUP_CREATE);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_lookup_followlink
 Input		:unsigned int flags
 		 < look up flags >
 Output		:void
 Return		:int
 		 < bool result >
 Description	:is a follow link flag set
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE int is_lookup_followlink(unsigned int flags)
{
	return(flags & LOOKUP_FOLLOW_LINK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_lookup_test
 Input		:unsigned int flags
 		 < look up flags >
 Output		:void
 Return		:int
 		 < bool result >
 Description	:is a follow link flag set
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE int is_lookup_test(unsigned int flags)
{
	return(flags & LOOKUP_TEST);
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

#endif	// __BK_FS_PATH_H__
