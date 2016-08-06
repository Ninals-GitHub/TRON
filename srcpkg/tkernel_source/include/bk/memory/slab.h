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

#ifndef	__BK_SLAB_H__
#define	__BK_SLAB_H__

#include <tstdlib/list.h>
#include <bk/gfp.h>


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
	slab
----------------------------------------------------------------------------------
*/
typedef unsigned int kmem_bufctl;

struct slab {
	struct list	list_slab;	/* list of slabs			*/
	unsigned long	color_offset;	/* offset of the first object		*/
	void		*s_mem;		/* pointer to objects			*/
	unsigned int	count;		/* count of used objects in a slab	*/
	kmem_bufctl	free;		/* first free index			*/
};

/*
----------------------------------------------------------------------------------
	cache management : kmem_cache
----------------------------------------------------------------------------------
*/
struct kmem_cache {
	/* -------------------------------------------------------------------- */
	/* kmem cache management						*/
	/* -------------------------------------------------------------------- */
	const char	*name;		/* name of the cache			*/
	struct list	list;		/* list of kmem caches			*/
	/* -------------------------------------------------------------------- */
	/* slab attributes							*/
	/* -------------------------------------------------------------------- */
	unsigned int	slab_size;	/* size of slab				*/
	int		align;		/* alignment				*/
	size_t		color_num;	/* number of colors			*/
	unsigned int	color_align;	/* color alignment offset		*/
	unsigned int	color_next;	/* next color				*/
	unsigned int	flags;		/* slab flags				*/
	
	void (*constructor)(void *obj);
	
	/* -------------------------------------------------------------------- */
	/* list of slab management						*/
	/* -------------------------------------------------------------------- */
	struct list	list_partial;	/* partially  used slabs		*/
	struct list	list_full;	/* fully used slabs			*/
	struct list	list_free;	/* completely free slbas		*/
	unsigned int	free_limit;	/* upper limit of free slabs		*/
	
	/* -------------------------------------------------------------------- */
	/* object attributes							*/
	/* -------------------------------------------------------------------- */
	size_t		obj_size;	/* size of a object			*/
	unsigned int	obj_num;	/* number of objects per slab		*/
	unsigned int	total_obj_num;	/* total number of objects		*/
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
 Funtion	:kmem_cache_create
 Input		:const char *name
 		 < name of a cache >
 		 size_t size
 		 < size of a object >
 		 unsigned int align
 		 < alignment >
 		 unsigned int flags
 		 < cache flags >
 		 void (*constructor)(void*)
 		 < function pointer to the construct >
 Output		:void
 Return		:struct kmem_cache*
 		 < created cache >
 Description	:create a cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct kmem_cache*
kmem_cache_create(const char *name, size_t size,
			unsigned int align, unsigned int flags,
			void (*constructor)(void*));

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmem_cache_alloc
 Input		:struct kmem_cache *cache
 		 < a cache to allocate from >
 		 unsigne int flags
 		 < cache flags >
 Output		:void
 Return		:void*
 		 < an allocated object >
 Description	:allocate an object from a cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void* kmem_cache_alloc(struct kmem_cache *cache, unsigned int flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmem_cache_zalloc
 Input		:struct kmem_cache *cache
 		 < a cache to allocate from >
 		 unsigne int flags
 		 < cache flags >
 Output		:void
 Return		:void*
 		 < an allocated object >
 Description	:allocate a zero cleared-object from a cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE
void* kmem_cache_zalloc(struct kmem_cache *cache, unsigned int flags)
{
	return(kmem_cache_alloc(cache, flags | __GFP_ZERO));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmem_cache_free
 Input		:struct kmem_cache *cache
 		 < cache from which allocated the object >
 		 void *ojbect
 		 < object to be freed >
 Output		:void
 Return		:void
 Description	:free an object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void kmem_cache_free(struct kmem_cache *cache, void *object);


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmem_cache_destroy
 Input		:struct kmem_cache *cache
 		 < cache to be destroyed >
 Output		:void
 Return		:void
 Description	:destroy a cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void kmem_cache_destroy(struct kmem_cache *cache);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_slab_allocator
 Input		:void
 Output		:void
 Return		:int
 		 < status >
 Description	:initialize slab allocator
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_slab_allocator(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmalloc
 Input		:size_t size
 		 < memory allocation size >
 		 unsigned int flags
 		 < flags to allocate memory >
 Output		:void
 Return		:void*
 		 < allocated memory address >
 Description	:allocate a kernel small buffer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void* kmalloc(size_t size, unsigned int flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kzalloc
 Input		:size_t size
 		 < memory allocation size >
 		 unsigned int flags
 		 < flags to allocate memory >
 Output		:void
 Return		:void*
 		 < allocated memory address >
 Description	:allocate a 0-clreared buffer 
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE void* kzalloc(size_t size, unsigned int flags)
{
	return(kmalloc(size, flags | __GFP_ZERO));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kcalloc
 Input		:size_t n
 		 < number of memory allocation >
 		 size_t size
 		 < memory allocation size >
 		 unsigned int flags
 		 < flags to allocate memory >
 Output		:void
 Return		:void*
 		 < allocated memory address >
 Description	:allocate a 0-clreared buffer 
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void* kcalloc(size_t n, size_t size, unsigned int flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kfree
 Input		:void *object
 		 < object to be freed >
 Output		:void
 Return		:void
 Description	:free a kernel small buffer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void kfree(void *object);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:krealloc
 Input		:void *object
 		 < object to be reallocated >
 		 size_t size
 		 < new size >
 		 unsigned int flags
 		 < flags to allocate memory >
 Output		:void
 Return		:void
 Description	:reallocate a kernel small buffer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void* krealloc(void *object, size_t size, unsigned int flags)
{
	kfree(object);
	return(kmalloc(size, flags));
}

#endif	// __BK_SLAB_H__
