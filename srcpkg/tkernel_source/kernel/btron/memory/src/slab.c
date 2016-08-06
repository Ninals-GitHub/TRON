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

#include <typedef.h>
#include <bk/memory/slab.h>
#include <bk/memory/page.h>
#include <t2ex/util.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int init_kmem_cache(struct kmem_cache *cache, const char *name, size_t size,
				unsigned int align, unsigned int flags,
				void (*constructor)(void*));
LOCAL int calc_slab_size(struct kmem_cache *cache);
LOCAL INLINE struct slab* get_slab_entry(struct list *list);
LOCAL INLINE struct slab* get_partial_slab(struct kmem_cache *cache);
LOCAL INLINE struct slab* get_free_slab(struct kmem_cache *cache);
LOCAL void* alloc_free_object(struct kmem_cache *cache, struct slab *slab);
LOCAL INLINE kmem_bufctl* get_kmem_bufctl(struct slab *slab);
LOCAL INLINE void*
get_object(struct kmem_cache *cache, struct slab *slab, unsigned int index);
LOCAL INLINE unsigned int
get_object_index(struct kmem_cache *cache, struct slab *slab, void *object);
LOCAL int kmem_cache_grow(struct kmem_cache *cache, unsigned int flags);
LOCAL INLINE int is_on_slab(struct kmem_cache *cache);
LOCAL INLINE int is_off_slab(struct kmem_cache *cache);
LOCAL INLINE void init_slab(struct slab *slab, void *s_mem);
LOCAL void init_kmem_bufctl(struct kmem_cache *cache, struct slab *slab);
LOCAL int kmem_cache_reap(struct kmem_cache *cache, struct list *list);
LOCAL int
setup_initial_cache(struct kmem_cache *cache, const char *name, size_t size);


/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define SLAB_LARGE_OBJECT_SIZE		(PAGESIZE / 8 )
#define	SLAB_MAX_PAGE_SIZE		(PAGESIZE << 13)

/*
----------------------------------------------------------------------------------
	slab flags
----------------------------------------------------------------------------------
*/
#define SLAB_FLAG_ON_SLAB		0x00000000
#define SLAB_FLAG_OFF_SLAB		0x00000001
#define SLAB_FLAG_INIT_CACHE		0x00000002

/*
----------------------------------------------------------------------------------
	bufctl
----------------------------------------------------------------------------------
*/
#define SLAB_KMEM_BUFCTL_END		(~0U)
#define SLAB_KMEM_BUFCTL_ALLOCATED	((~0U) - 1)

/*
----------------------------------------------------------------------------------
	caches for kmalloc
----------------------------------------------------------------------------------
*/
struct initial_cache {
	size_t		size;
	const char	name[15];
};

enum kmalloc_mem_size {
	KMALLOC_8,
	KMALLOC_16,
	KMALLOC_32,
	KMALLOC_64,
	KMALLOC_96,
	KMALLOC_128,
	KMALLOC_192,
	KMALLOC_256,
	KMALLOC_512,
	KMALLOC_1024,
	KMALLOC_2048,
	KMALLOC_4096,
	KMALLOC_8192,
	KMALLOC_16384,
	KMALLOC_32768,
	KMALLOC_65536,
	KMALLOC_131072,
	KMALLOC_262144,
	KMALLOC_NUM,
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct kmem_cache kmem_cache_cache;
LOCAL const char kmem_cache_name[] = "kmem_cache";
LOCAL struct list list_cache;

LOCAL const struct initial_cache initial_cache[KMALLOC_NUM] =
{
	{ 8,		"kmalloc-8"		},
	{ 16,		"kmalloc-16"		},
	{ 32,		"kmalloc-32"		},
	{ 64,		"kmalloc-64"		},
	{ 96,		"kmalloc-96"		},
	{ 128,		"kmalloc-128"		},
	{ 192,		"kmalloc-192"		},
	{ 256,		"kmalloc-256"		},
	{ 512,		"kmalloc-512"		},
	{ 1024,		"kmalloc-1024"		},
	{ 2048,		"kmalloc-2048"		},
	{ 4096,		"kmalloc-4096"		},
	{ 8192,		"kmalloc-8192"		},
	{ 16384,	"kmalloc-16384"		},
	{ 32768,	"kmalloc-32768"		},
	{ 65536,	"kmalloc-65536"		},
	{ 131072,	"kmalloc-131072"	},
	{ 262144,	"kmalloc-262144"	},
};

LOCAL struct kmem_cache kmalloc_cache[KMALLOC_NUM];

/* for T-Kernel									*/
EXPORT FastULock	_SmacbLock;	/* Smalloc lock */

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
EXPORT struct kmem_cache*
kmem_cache_create(const char *name, size_t size,
			unsigned int align, unsigned int flags,
			void (*constructor)(void*))
{
	struct kmem_cache *cache;
	int err;
	
	/* -------------------------------------------------------------------- */
	/* allocate a kmem_cache						*/
	/* -------------------------------------------------------------------- */
	cache = kmem_cache_alloc(&kmem_cache_cache, flags);
	
	if (!cache) {
		return(NULL);
	}
	
	/* -------------------------------------------------------------------- */
	/* initialize kmem_cache						*/
	/* -------------------------------------------------------------------- */
	err = init_kmem_cache(cache, name, size, align, flags, constructor);
	
	if (err) {
		kmem_cache_free(&kmem_cache_cache, cache);
		return(NULL);
	}
	
	/* -------------------------------------------------------------------- */
	/* add to cache list							*/
	/* -------------------------------------------------------------------- */
	add_list_tail(&cache->list, &list_cache);
	
	return(cache);
}

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
EXPORT void* kmem_cache_alloc(struct kmem_cache *cache, unsigned int flags)
{
	int err;
	struct slab *slab;
	void *allocated = NULL;
	
	/* -------------------------------------------------------------------- */
	/* test whether partial slabs are available or not			*/
	/* -------------------------------------------------------------------- */
	if (!is_empty_list(&cache->list_partial)) {
		slab = get_partial_slab(cache);
		//if (cache->name[0] == 'p')vd_printf("slab1-3:kmem_cache_alloc[%s]\n", cache->name);
		allocated = alloc_free_object(cache, slab);
		//if (cache->name[0] == 'p')vd_printf("slab2:kmem_cache_alloc[%s]\n", cache->name);
		if (allocated) {
			if (cache->obj_num <= slab->count) {
				/* -------------------------------------------- */
				/* delete an entry from partial list and move	*/
				/* to full list					*/
				/* -------------------------------------------- */
				move_list(&slab->list_slab, &cache->list_full);
			}
			
			goto object_allocated;
		}
		if (!allocated) {
			/* ---------------------------------------------------- */
			/* delete an entry from partial list and move to	*/
			/* to full list						*/
			/* ---------------------------------------------------- */
			move_list(&slab->list_slab, &cache->list_full);
		}
	}
	err = 0;
	/* -------------------------------------------------------------------- */
	/* check free list							*/
	/* -------------------------------------------------------------------- */
	if (is_empty_list(&cache->list_free)) {
		/* ------------------------------------------------------------ */
		/* create a new slab						*/
		/* ------------------------------------------------------------ */
		err = kmem_cache_grow(cache, flags);
	}
	
	if (!err) {
		/* ------------------------------------------------------------ */
		/* allocate a free object					*/
		/* ------------------------------------------------------------ */
		slab = get_free_slab(cache);
		
		allocated = alloc_free_object(cache, slab);
		
		if (allocated) {
			/* ---------------------------------------------------- */
			/* delete an entry from partial list and move		*/
			/* to partial list					*/
			/* ---------------------------------------------------- */
			move_list(&slab->list_slab, &cache->list_partial);
		}
	}
	
object_allocated:
	
	if (UNLIKELY(flags & __GFP_ZERO)) {
		memset(allocated, 0x00, cache->obj_size);
	}
	
	return(allocated);
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
EXPORT void kmem_cache_free(struct kmem_cache *cache, void *object)
{
	struct slab *slab;
	unsigned int obj_index;
	kmem_bufctl *pkmem_bufctl;
	struct page *page;
	
	/* -------------------------------------------------------------------- */
	/* get page to which an object belongs					*/
	/* -------------------------------------------------------------------- */
	page = get_page(object);
	
	if (UNLIKELY(page->flags != PAGE_SLAB)) {
		return;
	}
	
	slab = page->slab_page;
	
	if (UNLIKELY(!slab)) {
		return;
	}
	
	/* -------------------------------------------------------------------- */
	/* update kmem_bufctl							*/
	/* -------------------------------------------------------------------- */
	obj_index = get_object_index(cache, slab, object);
	
	pkmem_bufctl = get_kmem_bufctl(slab);
	
	if (pkmem_bufctl[obj_index] == SLAB_KMEM_BUFCTL_ALLOCATED) {
		pkmem_bufctl[obj_index] = slab->free;
		
		slab->free = obj_index;
		slab->count--;
		
		if (!slab->count) {
			move_list(&slab->list_slab, &cache->list_free);
		}
	}
}

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
EXPORT void kmem_cache_destroy(struct kmem_cache *cache)
{
	if (!cache) {
		return;
	}
	
	/* -------------------------------------------------------------------- */
	/* destroy slabs on partial list if exists				*/
	/* -------------------------------------------------------------------- */
	if (!is_empty_list(&cache->list_partial)) {
		kmem_cache_reap(cache, &cache->list_partial);
	}
	/* -------------------------------------------------------------------- */
	/* destroy slabs on full list if exists					*/
	/* -------------------------------------------------------------------- */
	if (!is_empty_list(&cache->list_full)) {
		kmem_cache_reap(cache, &cache->list_full);
	}
	/* -------------------------------------------------------------------- */
	/* destroy slabs on full list if exists					*/
	/* -------------------------------------------------------------------- */
	if (!is_empty_list(&cache->list_free)) {
		kmem_cache_reap(cache, &cache->list_free);
	}
	/* -------------------------------------------------------------------- */
	/* free objects								*/
	/* -------------------------------------------------------------------- */
	del_list(&cache->list);
	
	kmem_cache_free(&kmem_cache_cache, cache);
}

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
EXPORT int init_slab_allocator(void)
{
	int i;
	int err;
	int inited;

	/* -------------------------------------------------------------------- */
	/* init kmem_cache cache list						*/
	/* -------------------------------------------------------------------- */
	init_list(&list_cache);
	/* -------------------------------------------------------------------- */
	/* set up initial static caches						*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < KMALLOC_NUM;i++) {
		err = setup_initial_cache(&kmalloc_cache[i],
				initial_cache[i].name, initial_cache[i].size);
		
		if (UNLIKELY(err)) {
			inited = i;
			goto err_setup;
		}
	}
	inited = i - 1;
	/* -------------------------------------------------------------------- */
	/* set up static kmem_cache cache					*/
	/* -------------------------------------------------------------------- */
	err = setup_initial_cache(&kmem_cache_cache, kmem_cache_name,
						sizeof(kmem_cache_cache));
	
	if (UNLIKELY(err)) {
		goto err_setup;
	}
	
	return(err);
	
err_setup:
	for (i = inited;i;i--) {
		kmem_cache_destroy(&kmalloc_cache[i]);
	}
	
	return(err);
}

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
EXPORT void* kmalloc(size_t size, unsigned int flags)
{
	enum kmalloc_mem_size size_index;
	void *object;
	
	/* -------------------------------------------------------------------- */
	/* find the most appropriate cache size					*/
	/* -------------------------------------------------------------------- */
	for (size_index = KMALLOC_8;size_index < KMALLOC_NUM;size_index++) {
		/* ------------------------------------------------------------ */
		/* found it							*/
		/* ------------------------------------------------------------ */
		if (size <= initial_cache[size_index].size) {
			/* ---------------------------------------------------- */
			/* allocate an object from the cache			*/
			/* ---------------------------------------------------- */
			object = kmem_cache_alloc(&kmalloc_cache[size_index],
									flags);
			/* ---------------------------------------------------- */
			/* if the cache memory is exhausted			*/
			/* ---------------------------------------------------- */
			if (!object) {
				/* -------------------------------------------- */
				/* try next large size				*/
				/* -------------------------------------------- */
				continue;
			}
			
			return(object);
		}
	}
	return(NULL);
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
EXPORT void* kcalloc(size_t n, size_t size, unsigned int flags)
{
	void *object;
	
	object = kmalloc(size * n, flags);
	
	if (object) {
		memset(object, 0x00, size * n);
	}
	
	return(object);
}

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
EXPORT void kfree(void *object)
{
	struct page *page;
	struct kmem_cache *cache;
	
	if (UNLIKELY(!object)) {
		return;
	}
	
	/* -------------------------------------------------------------------- */
	/* get a page of slab							*/
	/* -------------------------------------------------------------------- */
	page = get_page(object);
	
	if (UNLIKELY(!page)) {
		return;
	}
	
	if (UNLIKELY(page->flags != PAGE_SLAB)) {
		return;
	}
	
	if (UNLIKELY(!page->slab_cache)) {
		return;
	}
	
	/* -------------------------------------------------------------------- */
	/* get a cache from a page						*/
	/* -------------------------------------------------------------------- */
	cache = page->slab_cache;
	/* -------------------------------------------------------------------- */
	/* free an object							*/
	/* -------------------------------------------------------------------- */
	kmem_cache_free(cache, object);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Imalloc
 Input		:size_t size
 		 < size of memory to allocate >
 Output		:void
 Return		:void
 Description	:allocate a memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Imalloc(size_t size)
{
	return(kmalloc(size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Icalloc
 Input		:size_t nmemb
 		 < number of memory areas >
 		 size_t size
 		 < size of memory area >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate a 0-cleared memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Icalloc(size_t nmemb, size_t size)
{
	return(kcalloc(nmemb, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Ifree
 Input		:void *ptr
 		 < pointer to object to be freed >
 Output		:void
 Return		:void
 Description	:free an object for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void Ifree(void *ptr)
{
	return(kfree(ptr));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:IAmalloc
 Input		:sizet_t size
 		 < size of memory to allocate >
 		 UINT attr
 		 < user attributes >
 Output		:void
 Return		:void
 Description	:allocate a memory with the attributes for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* IAmalloc(size_t size, UINT attr)
{
	if (attr & TA_RNG3) {
		/* ------------------------------------------------------------ */
		/* as for now, allocation for user memory is not supported	*/
		/* in the future implemented to call mmap function		*/
		/* ------------------------------------------------------------ */
		return(NULL);
	} else {
		return(kmalloc(size, 0));
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:IAcalloc
 Input		:size_t nmemb
 		 < number of memory areas >
 		 size_t size
 		 < size of memory areas >
 		 UINT attr
 		 < user attributes >
 Output		:void
 Return		:void
 Description	:allocate a 0-cleared memory with the attributes for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* IAcalloc(size_t nmemb, size_t size, UINT attr)
{
	if (attr & TA_RNG3) {
		/* ------------------------------------------------------------ */
		/* as for now, allocation for user memory is not supported	*/
		/* in the future implemented to call mmap function		*/
		/* ------------------------------------------------------------ */
		return(NULL);
	} else {
		return(kcalloc(nmemb, size, 0));
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:IAfree
 Input		:void *ptr
 		 < pointer to object to be freed >
 		 UINT attr
 		 < user attributes >
 Output		:void
 Return		:void
 Description	:free an object with the attributes for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void IAfree(void *ptr, UINT attr)
{
	if (attr & TA_RNG3) {
		/* ------------------------------------------------------------ */
		/* as for now, allocation for user memory is not supported	*/
		/* in the future implemented to call unmmap function		*/
		/* ------------------------------------------------------------ */
		/* do nothing							*/
	} else {
		kfree(ptr);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Smalloc
 Input		:size_t size
 		 < size of memory to allocate >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate system memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Smalloc(size_t size)
{
	return(kmalloc(size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Scalloc
 Input		:size_t nmemb
 		 < number of memory areas >
 		 size_t size
 		 < size of memory area >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate a 0-cleared system memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Scalloc(size_t nmemb, size_t size)
{
	return(kcalloc(nmemb, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Srealloc
 Input		:void *ptr
 		 < pointer to object to reallocate a memory >
 		 size_t size
 		 < rellocation size >
 Output		:void
 Return		:void*
 		 < reallocated memory >
 Description	:reallocate a system memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Srealloc(void *ptr, size_t size)
{
	return(krealloc(ptr, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Sfree
 Input		:void *ptr
 		 < memory area to be freed >
 Output		:void
 Return		:void
 Description	:free a system memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void Sfree(void *ptr)
{
	kfree(ptr);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Vmalloc
 Input		:size_t size
 		 < size of memory to allocate >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate a non-resident memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Vmalloc(size_t size)
{
	return(kmalloc(size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Vcalloc
 Input		:size_t nmemb
 		 < number of memory areas >
 		 size_t size
 		 < size of memory area >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate a non-resident memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Vcalloc(size_t nmemb, size_t size)
{
	return(kcalloc(nmemb, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Vrealloc
 Input		:void *ptr
 		 < pointer to object to reallocate >
 		 size_t size
 		 < new memory size >
 Output		:void
 Return		:void*
 		 < rellocated memory >
 Description	:reallocate a non-resident memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Vrealloc(void *ptr, size_t size)
{
	return(krealloc(ptr, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Vfree
 Input		:void *ptr
 		 < pointer to object to free >
 Output		:void
 Return		:void
 Description	:free a non-resident memory for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void Vfree(void *ptr)
{
	kfree(ptr);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Kmalloc
 Input		:size_t size
 		 < size of memory to allocate >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate a kernel small buffer for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Kmalloc(size_t size)
{
	return(kmalloc(size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Kcalloc
 Input		:size_t nmemb
 		 < number of memory areas >
 		 size_t size
 		 < size of memory area >
 Output		:void
 Return		:void*
 		 < allocated memory >
 Description	:allocate a kernel small buffer for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Kcalloc(size_t nmemb, size_t size)
{
	return(kcalloc(nmemb, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Krealloc
 Input		:void *ptr
 		 < pointer to object to reallocate >
 		 size_t size
 		 < new memory size >
 Output		:void
 Return		:void*
 		 < rellocated memory >
 Description	:reallocate a kernel small buffer for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* Krealloc(void *ptr, size_t size)
{
	return(krealloc(ptr, size, 0));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:Kfree
 Input		:void *ptr
 		 < pointer to object to free >
 Output		:void
 Return		:void
 Description	:free a kernel small buffer for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void Kfree(void *ptr)
{
	kfree(ptr);
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
 Funtion	:init_kmem_cache
 Input		:struct kmem_cache *cache
 		 < initializee >
 		 const char *name
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
 Return		:int
 		 < status >
 Description	:initialize kmem_cache
==================================================================================
*/
LOCAL int init_kmem_cache(struct kmem_cache *cache, const char *name, size_t size,
				unsigned int align, unsigned int flags,
				void (*constructor)(void*))
{
	int err;

	/* -------------------------------------------------------------------- */
	/* initialize a kmem cache management					*/
	/* -------------------------------------------------------------------- */
	init_list(&cache->list);
	cache->name = name;
	/* -------------------------------------------------------------------- */
	/* initialize object attribute						*/
	/* -------------------------------------------------------------------- */
	cache->obj_size = size;
	cache->total_obj_num = 0;
	/* -------------------------------------------------------------------- */
	/* initialize slab attribute						*/
	/* -------------------------------------------------------------------- */
	cache->flags = 0;
	/* set off-slab flag							*/
	if (SLAB_LARGE_OBJECT_SIZE <= cache->obj_size) {
		cache->flags |= SLAB_FLAG_OFF_SLAB;
	}
	/* -------------------------------------------------------------------- */
	/* calculate size of slab and number of objects in a cache		*/
	/* -------------------------------------------------------------------- */
	err = calc_slab_size(cache);
	
	if (err) {
		vd_printf("%s:calc_slab_size\n", __func__);
		return(err);
	}
	
	cache->constructor = constructor;
	
	/* -------------------------------------------------------------------- */
	/* initialize list of slab management					*/
	/* -------------------------------------------------------------------- */
	init_list(&cache->list_partial);
	init_list(&cache->list_full);
	init_list(&cache->list_free);
	
	return(0);
}

/*
==================================================================================
 Funtion	:calc_slab_size
 Input		:struct kmem_cache*
 Output		:struct kmem_cache*
 		 < slab_size, obj_num >
 Return		:int
 		 < status >
 Description	:calculate slab size in byte kmem_cache must be set flags,
 		 obj_size and align appropriately
==================================================================================
*/
LOCAL int calc_slab_size(struct kmem_cache *cache)
{
	size_t slab_size;
	int obj_num;
	/* -------------------------------------------------------------------- */
	/* on-slab								*/
	/* -------------------------------------------------------------------- */
	if (is_on_slab(cache)) {
		slab_size = PAGESIZE;
		obj_num = slab_size - sizeof(struct slab);
		obj_num /= cache->obj_size + sizeof(kmem_bufctl);
	} else {
	/* -------------------------------------------------------------------- */
	/* off-slab								*/
	/* -------------------------------------------------------------------- */
		for (slab_size = (PAGESIZE << 1);
			slab_size < SLAB_MAX_PAGE_SIZE;
			slab_size = slab_size << 1) {
			obj_num = slab_size / cache->obj_size;
			/* ---------------------------------------------------- */
			/* size of slab is determined				*/
			/* ---------------------------------------------------- */
			if (obj_num) {
				break;
			}
		}
	}
	/* -------------------------------------------------------------------- */
	/* too large slab							*/
	/* -------------------------------------------------------------------- */
	if (!obj_num) {
		cache->slab_size = 0;
		cache->obj_num = 0;
		return(-1);
	}
	
	cache->slab_size = slab_size;
	cache->obj_num = obj_num;
	
	return(0);
}

/*
==================================================================================
 Funtion	:get_slab_entry
 Input		:struct list *list
 		 < list of kmem_cache list_free, list_partial, list_full >
 Output		:void
 Return		:struct slab*
 		 < slab which belongs to a list >
 Description	:get a slab from a list
==================================================================================
*/
LOCAL INLINE struct slab* get_slab_entry(struct list *list)
{
	return(get_first_entry(list, struct slab, list_slab));
}

/*
==================================================================================
 Funtion	:get_partial_slab
 Input		:struct kmem_cache *cache
 		 < cache to which a slab belongs >
 Output		:void
 Return		:struct slab*
 		 < partial slab >
 Description	:get a partial slab
==================================================================================
*/
LOCAL INLINE struct slab* get_partial_slab(struct kmem_cache *cache)
{
	return(get_slab_entry(&cache->list_partial));
}

/*
==================================================================================
 Funtion	:get_free_slab
 Input		:struct kmem_cache *cache
 		 < cache to which a slab belongs >
 Output		:void
 Return		:struct slab*
 		 < free slab >
 Description	:get a free slab
==================================================================================
*/
LOCAL INLINE struct slab* get_free_slab(struct kmem_cache *cache)
{
	return(get_slab_entry(&cache->list_free));
}

/*
==================================================================================
 Funtion	:alloc_free_object
 Input		:struct kmem_cache *cache
 		 < cache from which allocates a object >
 		 struct slab *slab
 		 < slab from which allocates a object >
 Output		:void
 Return		:void*
 		 < object address >
 Description	:allocate a free object
==================================================================================
*/
LOCAL void* alloc_free_object(struct kmem_cache *cache, struct slab *slab)
{
	kmem_bufctl free;
	kmem_bufctl *kmem_bufctl;
	
	if (slab->free != SLAB_KMEM_BUFCTL_END) {
		/* ------------------------------------------------------------ */
		/* update kmem_bufctl						*/
		/* ------------------------------------------------------------ */
		kmem_bufctl = get_kmem_bufctl(slab);
		free = slab->free;
		//if (cache->name[0] == 'p'){
		//	vd_printf("slab2-3:kmem_cache_alloc[%s]\n", cache->name);
		//	vd_printf("current:0x%08X 0x%08X", slab->free, slab);
		//}
		slab->free = kmem_bufctl[slab->free];
		//if (cache->name[0] == 'p'){
		//	vd_printf(" next:0x%08X 0x%08X\n", slab->free, slab);
		//	vd_printf("slab2-4:kmem_cache_alloc[%s]\n", cache->name);
		//}
		kmem_bufctl[free] = SLAB_KMEM_BUFCTL_ALLOCATED;
		slab->count++;
		return(get_object(cache, slab, free));
	}
	
	return(NULL);
}

/*
==================================================================================
 Funtion	:get_kmem_bufctl
 Input		:struct slab *slab
 		 < kmem_bufctl is right after this slab >
 Output		:void
 Return		:kmem_bufctl
 		 < address of kmem_bufctl
 Description	:get address of kmem_bufctl
==================================================================================
*/
LOCAL INLINE kmem_bufctl* get_kmem_bufctl(struct slab *slab)
{
	return((kmem_bufctl*)(slab + 1));
}

/*
==================================================================================
 Funtion	:get_object
 Input		:struct kmem_cahce *cache
 		 < cache to which a slab belongs >
 		 struct slab *slab
 		 < slab to which a object belongs >
 		 unsgined int index
 		 < index of the object >
 Output		:void
 Return		:void*
 		 < address of an object >
 Description	:get object address at index
==================================================================================
*/
LOCAL INLINE void*
get_object(struct kmem_cache *cache, struct slab *slab, unsigned int index)
{
	return((void*)((unsigned long)slab->s_mem + cache->obj_size * index));
}

/*
==================================================================================
 Funtion	:get_object_index
 Input		:struct kmem_cache *cache
 		 < cache to which a slab belongs >
 		 struct slab *slab
 		 < slab to which an object belongs >
 		 void *object
 		 < object address >
 Output		:void
 Return		:unsigned int
 		 < index of object in a slab >
 Description	:get index of an object
==================================================================================
*/
LOCAL INLINE unsigned int
get_object_index(struct kmem_cache *cache, struct slab *slab, void *object)
{
	unsigned long start_object;
	
	start_object = (unsigned long)get_object(cache, slab, 0);
	
	return(((unsigned long)object - start_object) / cache->obj_size);
}

/*
==================================================================================
 Funtion	:kmem_cache_grow
 Input		:struct kmem_cache *cache
 		 < cache to be grown >
 		 unsigned int flags
 		 < flags >
 Output		:void
 Return		:int
 		 < status >
 Description	:create a slab and add it to a cache
==================================================================================
*/
LOCAL int kmem_cache_grow(struct kmem_cache *cache, unsigned int flags)
{
	struct slab *slab;
	struct page *page;
	void *s_mem;
	kmem_bufctl *pkmem_bufctl;
	int i;
	void *object;
	size_t nr_pages;
	
	/* -------------------------------------------------------------------- */
	/* off-slab:allocate multiple pages					*/
	/* -------------------------------------------------------------------- */
	if (is_off_slab(cache)) {
		slab = (struct slab*)kmalloc(sizeof(struct slab) +
					sizeof(kmem_bufctl) * cache->obj_num, 0 );
		
		if (!slab) {
			return(-1);
		}
		
		nr_pages = PageCount(cache->slab_size);
		
		page = alloc_slab_pages(nr_pages);
		
		if (!page) {
			kfree(slab);
			return(-1);
		}
		
		s_mem = (void*)page_to_address(page);
		
	} else {
	/* -------------------------------------------------------------------- */
	/* on-slab:allocate a page						*/
	/* -------------------------------------------------------------------- */
		nr_pages = 1;
		page = (struct page*)alloc_slab_pages(nr_pages);
		
		if (!page) {
			vd_printf("%s:cannot alloc_slab_pages\n", __func__);
			return(-1);
		}
		
		slab = (struct slab*)page_to_address(page);
		
		/* ------------------------------------------------------------ */
		/* kmem_bufctl is located right after a slab			*/
		/* ------------------------------------------------------------ */
		pkmem_bufctl = get_kmem_bufctl(slab);
		/* ------------------------------------------------------------ */
		/* s_mem is located right after kmem_bufctl			*/
		/* ------------------------------------------------------------ */
		s_mem = (void*)(pkmem_bufctl + cache->obj_num);
	}
	/* -------------------------------------------------------------------- */
	/* initialize a slab management						*/
	/* -------------------------------------------------------------------- */
	init_slab(slab, s_mem);
	/* -------------------------------------------------------------------- */
	/* initialize kmem_bufctl						*/
	/* -------------------------------------------------------------------- */
	init_kmem_bufctl(cache, slab);
	/* -------------------------------------------------------------------- */
	/* add a slab to free list						*/
	/* -------------------------------------------------------------------- */
	add_list_tail(&slab->list_slab, &cache->list_free);
	/* -------------------------------------------------------------------- */
	/* grow total number of objects						*/
	/* -------------------------------------------------------------------- */
	cache->total_obj_num += cache->obj_num;
	/* -------------------------------------------------------------------- */
	/* initialize objects							*/
	/* -------------------------------------------------------------------- */
	if (cache->constructor) {
		for (i = 0;i < cache->obj_num;i++) {
			object = get_object(cache, slab, i);
			cache->constructor(object);
		}
	}
	/* -------------------------------------------------------------------- */
	/* initialize page infromation						*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < nr_pages;i++) {
		//page[i].s_mem = s_mem;
		page[i].slab_page = slab;
		page[i].slab_cache = cache;
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:is_on_slab
 Input		:struct kmem_cache *cache
 		 < cache to test >
 Output		:void
 Return		:int
 		 < 0:is not on slab 1:is on slab
 Description	:test whether a slab is on-slab
==================================================================================
*/
LOCAL INLINE int is_on_slab(struct kmem_cache *cache)
{
	return(SLAB_FLAG_ON_SLAB == (cache->flags & SLAB_FLAG_OFF_SLAB));
}

/*
==================================================================================
 Funtion	:is_ff_slab
 Input		:struct kmem_cache *cache
 		 < cache to test >
 Output		:void
 Return		:int
 		 < 0:is not off slab 1:is off slab
 Description	:test whether a slab is off-slab
==================================================================================
*/
LOCAL INLINE int is_off_slab(struct kmem_cache *cache)
{
	return(cache->flags & SLAB_FLAG_OFF_SLAB);
}

/*
==================================================================================
 Funtion	:init_slab
 Input		:struct slab *slab
 		 < slab to be initialized >
 		 void *s_mem
 		 < memory address of objects >
 Output		:struct slab *slab
 		 < initializee >
 Return		:void
 Description	:initialize a slab
==================================================================================
*/
LOCAL INLINE void init_slab(struct slab *slab, void *s_mem)
{
	init_list(&slab->list_slab);
	slab->s_mem = s_mem;
	slab->count = 0;
}

/*
==================================================================================
 Funtion	:init_kmem_bufctl
 Input		:struct kmem_cache *cache
 		 < cache to which a slab belongs >
 		 struct slab *slab
 		 < initialize kmem_bufctl right after this slab >
 Output		:void
 Return		:void
 Description	:initialize a kmem_bufctl
==================================================================================
*/
LOCAL void init_kmem_bufctl(struct kmem_cache *cache, struct slab *slab)
{
	int i;
	kmem_bufctl *kmem_bufctl;
	
	/* -------------------------------------------------------------------- */
	/* get address of kmem_bufctl						*/
	/* -------------------------------------------------------------------- */
	kmem_bufctl = get_kmem_bufctl(slab);
	/* -------------------------------------------------------------------- */
	/* make a free list for objects						*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < (cache->obj_num - 1);i++) {
		kmem_bufctl[i] = i + 1;
	}
	/* -------------------------------------------------------------------- */
	/* set end of free list							*/
	/* -------------------------------------------------------------------- */
	kmem_bufctl[cache->obj_num - 1] = SLAB_KMEM_BUFCTL_END;
	/* -------------------------------------------------------------------- */
	/* set first free object index						*/
	/* -------------------------------------------------------------------- */
	slab->free = 0;
}

/*
==================================================================================
 Funtion	:kmem_cache_reap
 Input		:struct kmem_cache *cache
 		 < cache to which a slab belongs >
 		 struct list *list
 		 < slab list to be reaped >
 Output		:void
 Return		:void
 Description	:destroy all slabs
==================================================================================
*/
LOCAL int kmem_cache_reap(struct kmem_cache *cache, struct list *list)
{
	struct list *pos;
	struct slab *slab;
	struct page *page;
	
	/* -------------------------------------------------------------------- */
	/* iterate list to free slabs						*/
	/* -------------------------------------------------------------------- */
	list_for_each(pos, list) {
		slab = get_entry(pos, struct slab, list_slab);
		
		page = get_page((void*)slab);
		
		/* ------------------------------------------------------------ */
		/* on-slab:free only the slab itself				*/
		/* ------------------------------------------------------------ */
		if (is_on_slab(cache)) {
			free_pages(page, 1);
		} else {
		/* ------------------------------------------------------------ */
		/* off-slab:free s_mem and slab information			*/
		/* ------------------------------------------------------------ */
			int nr_pages;
			struct kmem_cache *cache_p = page->slab_cache;
			nr_pages = PageCount(cache_p->slab_size);
			free_pages(page, nr_pages);
			kfree((void*)slab);
		}
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:setup_initial_cache
 Input		:struct kmem_cache *cache
 		 < cache to be setup >
 		 const char *name
 		 < name of a cache >
 		 size_t size
 		 < size of an object >
 Output		:struct kmem_cache *cache
 Return		:int
 		 < status >
 Description	:void
==================================================================================
*/
LOCAL int
setup_initial_cache(struct kmem_cache *cache, const char *name, size_t size)
{
	int err;
	/* -------------------------------------------------------------------- */
	/* initialize an initial cache	 					*/
	/* -------------------------------------------------------------------- */
	err = init_kmem_cache(cache, name, size, 0, 0, NULL);
	
	if (UNLIKELY(err)) {
		vd_printf("init_kmem_cache %s\n", name);
		return(err);
	}
	/* -------------------------------------------------------------------- */
	/* calculate size of slab and number of objects in a cache	 	*/
	/* -------------------------------------------------------------------- */
	err = calc_slab_size(cache);
	
	if (UNLIKELY(err)) {
		vd_printf("calc_slab_size\n");
		return(err);
	}
	/* -------------------------------------------------------------------- */
	/* add to cache list						 	*/
	/* -------------------------------------------------------------------- */
	add_list_tail(&cache->list, &list_cache);
	/* -------------------------------------------------------------------- */
	/* create a on-slab						 	*/
	/* -------------------------------------------------------------------- */
	if (is_on_slab(cache)) {
		err = kmem_cache_grow(cache, 0);
		if (UNLIKELY(err)) {
			vd_printf("kmem_cache_grow\n");
		}
	}
	
	return(err);
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
