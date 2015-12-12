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


#include <bk/bprocess.h>
#include <bk/memory/mmap.h>
#include <bk/memory/vm.h>
#include <bk/memory/page.h>
#include <bk/uapi/berrno.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL INLINE void _free_vm(struct process *proc);
LOCAL INLINE void __free_vm(struct vm *vm);

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
LOCAL struct kmem_cache *mspace_cache;
LOCAL const char mspace_cache_name[] = "mspace_cache";

LOCAL struct kmem_cache *vm_cache;
LOCAL const char vm_cache_name[] = "vm_cache";

#if 0
LOCAL struct kmem_cache *vm_pages_cache;
LOCAL const char vm_pages_cache_name[] = "vm_pages_cache";
#endif

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_mm
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize virtuam memory management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_mm(void)
{
	mspace_cache = kmem_cache_create(mspace_cache_name,
					sizeof(struct memory_space), 0, 0, NULL);
	
	if (!mspace_cache) {
		vd_printf("error:mspace_cache\n");
		return(-ENOMEM);
	}

	vm_cache = kmem_cache_create(vm_cache_name, sizeof(struct vm), 0, 0, NULL);
	
	if (!vm_cache) {
		vd_printf("error:vm_cache\n");
		return(-ENOMEM);
	}
	
#if 0
	vm_pages_cache =kmem_cache_create(vm_pages_cache_name,
					sizeof(struct vm_pages), 0, 0, NULL);
	
	if (!vm_pages_cache) {
		vd_printf("error:vm_pages_cache\n");
		goto err_create_cache;
	}
#endif
	
	return(0);
#if 0
err_create_cache:
	kmem_cache_destroy(vm_cache);
	return(-ENOMEM);
#endif
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_memory_space
 Input		:void
 Output		:void
 Return		:struct memory_space*
 		 < memory space for user space >
 Description	:initialize memory space on space spawning
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct memory_space* alloc_memory_space(void)
{
	struct memory_space *mspace;
	
	mspace = (struct memory_space*)kmem_cache_alloc(mspace_cache, 0);
	
	if (!mspace) {
		return(NULL);
	}
	
	memset((void*)mspace, 0x00, sizeof(struct memory_space));
	
	init_list(&mspace->list_vm);
	
	return(mspace);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_memory_space
 Input		:struct memory_space *mspace
 		 < memory space >
 Output		:void
 Return		:void
 Description	:free memory space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_memory_space(struct memory_space *mspace)
{
	kmem_cache_free(mspace_cache, (void*)mspace);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_vm
 Input		:void
 Output		:void
 Return		:struct vm*
 		 < vm struct >
 Description	:allocate a vm struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vm*
//alloc_vm(unsigned long start, unsigned long end, unsigned int prot)
alloc_vm(void)
{
	struct vm *vm = (struct vm*)kmem_cache_alloc(vm_cache, 0);
	
	init_list(&vm->link_vm);
	
	vm->pages = NULL;
	vm->nr_pages = 0;
	vm->start = 0;
	vm->end = 0;
	
	return(vm);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vm_all
 Input		:struct process *proc
 		 < process which has vm areas >
 Output		:void
 Return		:void
 Description	:free virtual memory and all page tables including kernel space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_vm_all(struct process *proc)
{
	/* -------------------------------------------------------------------- */
	/* free all page tables and page directory				*/
	/* -------------------------------------------------------------------- */
	free_pagedir_tables(proc->mspace->pde);
	/* -------------------------------------------------------------------- */
	/* free virtual memory for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_vm(proc);

}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vm
 Input		:struct process *proc
 		 < process which has vm areas >
 Output		:void
 Return		:void
 Description	:free virtual memory for a user process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_vm(struct process *proc)
{
	/* -------------------------------------------------------------------- */
	/* free user space page tables						*/
	/* -------------------------------------------------------------------- */
	free_pagetables(proc->mspace->pde);
	/* -------------------------------------------------------------------- */
	/* free virtual memory for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_vm(proc);

}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_vm_pages
 Input		:int nr_pages
 		 < number of pages >
 Output		:void
 Return		:struct pages**
 		 < pages struct array >
 Description	:allocate a vm_pages struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page** alloc_vm_pages(int nr_pages)
{
	struct page **vp;
	
	vp = (struct page**)kmalloc(sizeof(struct page*) * nr_pages, 0);
	
	return(vp);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vm_pages
 Input		:struct vm *vm
 		 < vm which has pages >
 Output		:void
 Return		:void
 Description	:free pages allocated for virtual memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_vm_pages(struct vm *vm)
{
	vm->nr_pages = 0;
	kfree((void*)vm->pages);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:insert_vm_list
 Input		:struct vm *new
 		 < an insertee >
 		 struct process *proc
 		 < a process to be inserted a vm >
 Output		:void
 Return		:int
 		 < result >
 Description	:insert vm to a vm space of a process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int insert_vm_list(struct vm *new, struct process *proc)
{
	struct list *pos;
	struct vm *proc_vm;
	struct vm *proc_vm_next;
	struct vm *proc_vm_prev;
	
	if (is_empty_list(&proc->mspace->list_vm)) {
		add_list(&new->link_vm, &proc->mspace->list_vm);
		return(0);
	}
	
	list_for_each(pos, &proc->mspace->list_vm) {
		proc_vm = get_entry(pos, struct vm, link_vm);
		
		if (UNLIKELY(pos->next == &proc->mspace->list_vm)) {
			add_list(&new->link_vm, pos);
			return(0);
		}
		
		proc_vm_next = get_entry(pos->next, struct vm, link_vm);
		proc_vm_prev = get_entry(pos->prev, struct vm, link_vm);
		
		if ((proc_vm_prev->end <= new->start) &&
				(new->end <= proc_vm->start)) {
			add_list(&new->link_vm, pos->prev);
		}
		
		if ((proc_vm->end <= new->start) &&
			(new->end <= proc_vm_next->start)) {
			add_list(&new->link_vm, pos);
			return(0);
		}
	}
	
	return(-1);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_address_vm
 Input		:struct process *proc
 		 < get vm from the process >
 		 unsigned long address
 		 < user address which belongs to vm of process >
 Output		:void
 Return		:struct vm*
 		 < vm to which address belongs >
 Description	:get vm to which address belongs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vm* get_address_vm(struct process *proc, unsigned long address)
{
	struct vm *vm;
	
	list_for_each_entry(vm, &proc->mspace->list_vm, link_vm) {
		if (vm->start <= address && address <= vm->end) {
			return(vm);
		}
	}
	return(NULL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_mm
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy memory_space struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_mm(struct process *from, struct process *to)
{
	int err;
	
	if (!to->mspace) {
		return(-ENOMEM);
	}
	
	err = copy_pagetable(from, to);
	
	if (err) {
		goto copy_pagetable_failed;
	}
	
	err = copy_vm(from, to);
	
	if (err) {
		goto copy_vm_failed;
	}
	
copy_vm_failed:
	free_pagedir_tables(to->mspace->pde);

copy_pagetable_failed:
	free_memory_space(to->mspace);
	
	return(-ENOMEM);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_vm
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy vm struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_vm(struct process *from, struct process *to)
{
	struct vm *vm_from;
	int err;
	
	list_for_each_entry(vm_from, &from->mspace->list_vm, link_vm) {
		struct vm *vm_to = alloc_vm();
		
		if (!vm_to) {
			return(-ENOMEM);
		}
		
		vm_to->start = vm_from->start;
		vm_to->end = vm_from->end;
		/* ------------------------------------------------------------ */
		/* if write prot						*/
		/* ------------------------------------------------------------ */
		if (vm_from->prot & VM_WRITE) {
			vm_from->prot &= ~VM_WRITE;
			vm_from->prot |= VM_COW;
		}
		vm_to->prot = vm_from->prot;
		
		/* ------------------------------------------------------------ */
		/* copy page link information					*/
		/* ------------------------------------------------------------ */
		err = copy_vm_pages(vm_from, vm_to);
		
		if (err) {
			return(err);
		}
		/* ------------------------------------------------------------ */
		/* add it to process						*/
		/* ------------------------------------------------------------ */
		add_list_tail(&vm_to->link_vm, &to->mspace->list_vm);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_vm_pages
 Input		:struct vm *vm_from
 		 < copy from >
 		 struct vm *vm_to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy vm_pages struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_vm_pages(struct vm *vm_from, struct vm *vm_to)
{
	struct page **pages;
	int i;
	
	pages = alloc_vm_pages(vm_from->nr_pages);
	
	if (!pages) {
		return(-ENOMEM);
	}
	
	for (i = 0;i < vm_from->nr_pages;i++) {
		pages[i] = vm_from->pages[i];
	}
	
	vm_to->pages = pages;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:xvm_map_vm
 Input		:struct process *proc
 		 < a process to create its vm >
 		 unsigned long mmap_start
 		 < start address of user space >
 		 unsigned long mmap_end
 		 < end address of user space >
 		 unsigned int prot
 		 < permission >
 		 int anon
 		 < boolean : anonymous mappiing or not >
 Output		:void
 Return		:int
 		 < result >
 Description	:map a virtual memory for user space
 		 this function is currently used for test only
 		 future work;
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int xvm_map_vm(struct process *proc,
			unsigned long mmap_start, unsigned long mmap_end,
			unsigned int prot, int anon)
{
	int err;
	int i, err_i;
	struct vm *vm = alloc_vm();
	
	if (!vm) {
		printf("map_vm failed:1\n");
		return(-ENOMEM);
	}
	
	if (mmap_end <= mmap_start) {
		printf("map_vm failed:2\n");
		return(-EINVAL);
	}
	
	vm->start = mmap_start & PAGE_MASK;
	vm->end = RoundPage(mmap_end);
	vm->prot = prot;
	vm->nr_pages = PageCount(mmap_end - mmap_start);
	
	vm->pages = alloc_vm_pages(vm->nr_pages);
	
	if (!vm->pages) {
		err = -ENOMEM;
		printf("map_vm failed:3\n");
		goto failed_alloc_vm_pages;
	}
	
	vm->prot = prot;
	
	for (i = 0;i < vm->nr_pages;i++) {
		if (anon) {
			vm->pages[i] = alloc_zeroed_page();
		} else {
			vm->pages[i] = alloc_page();
		}
		
		if (!vm->pages[i]) {
			err_i = i;
			err = -ENOMEM;
			printf("map_vm failed:4\n");
			goto failed_alloc_page;
		}
	}
	
	err_i = i;
	
	err = map_pages_to_vm(proc, vm);
	
	if (err) {
		printf("map_vm failed:5\n");
		goto failed_map_pages_to_vm;
	}
	
	err = insert_vm_list(vm, proc);
	
	if (err) {
		printf("map_vm failed:6\n");
		err = -EINVAL;
		goto failed_insert_vm;
	}
	
	return(0);
	
failed_insert_vm:
failed_map_pages_to_vm:
failed_alloc_page:
	for (i = 0;i < err_i;i++) {
		free_page(vm->pages[i]);
	}
failed_alloc_vm_pages:

	__free_vm(vm);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unmap_vm
 Input		:struct process *proc
 		 < a process to unmap its vm >
 		 unsigned long mmap_start
 		 < start address to unmap >
 		 unsigned long mmap_end
 		 < end address to unmap >
 Output		:void
 Return		:int
 		 < result >
 Description	:unmap vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int unmap_vm(struct process *proc,
			unsigned long mmap_start, unsigned long mmap_end)
{
	struct memory_space *mspace = proc->mspace;
	struct vm *vm;
	struct vm *temp;
	struct vm *prev;
	struct vm *next;
	int err = 0;
	
	if (is_empty_list(&mspace->list_vm)) {
		return(err);
	}
	
	list_for_each_entry_safe(vm, temp, &proc->mspace->list_vm, link_vm) {
		if (vm->start == mmap_start && mmap_end == vm->end) {
			if (vm->nr_pages && vm->pages) {
				free_vm_pages(vm);
			}
			__free_vm(vm);
			return(err);
		} else if (vm->start < mmap_start && mmap_end < vm->end) {
		/* ------------------------------------------------------------ */
		/* split vm area						*/
		/* ------------------------------------------------------------ */
			unsigned long nr_prev;
			unsigned long nr_next;
			unsigned long nr_del;
			unsigned long i;
			unsigned long index;
			
			prev = alloc_vm();
			
			if (!prev) {
				return(-ENOMEM);
			}
			
			next = alloc_vm();
			
			if (!next) {
				err = -ENOMEM;
				goto failed_alloc_vm_next;
			}
			
			prev->start = vm->start;
			prev->end = mmap_start;
			next->start = mmap_end;
			next->end = vm->end;
			
			nr_prev = PageCount(prev->end - prev->start);
			nr_next = PageCount(next->end - next->start);
			nr_del = PageCount(next->start - prev->end);
			
			prev->pages = alloc_vm_pages(nr_prev);
			
			if (!prev) {
				err = -ENOMEM;
				goto failed_prev;
			}
			
			next->pages = alloc_vm_pages(nr_next);
			
			if (!next) {
				err = -ENOMEM;
				goto failed_next;
			}
			
			
			/* ---------------------------------------------------- */
			/* copy pages to prev area				*/
			/* ---------------------------------------------------- */
			for (i = 0;i < nr_prev;i++) {
				prev->pages[i] = vm->pages[i];
			}
			
			index = i;
			/* ---------------------------------------------------- */
			/* free pages of split area				*/
			/* ---------------------------------------------------- */
			for (;i < index + nr_del;i++) {
				free_page(vm->pages[i]);
			}
			
			index = i;
			/* ---------------------------------------------------- */
			/* copy pages to next area				*/
			/* ---------------------------------------------------- */
			for (;i < index + nr_next;i++) {
				next->pages[i - index] = vm->pages[i];
			}
			
			add_list_element(&prev->link_vm,
					vm->link_vm.prev, &vm->link_vm);
			
			add_list(&next->link_vm, &vm->link_vm);
			
			del_list(&vm->link_vm);
		}
	}
	
	return(err);
	
failed_next:
	free_vm_pages(prev);
failed_prev:
	__free_vm(next);
failed_alloc_vm_next:
	__free_vm(prev);
	return(err);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:search_mmap_candidate
 Input		:struct process *proc
 		 < process to search its vm >
 		 size_t length
 		 < size of memory to request a mapping >
 		 int prot
 		 < page protection >
 		 int flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor >
 		 off_t offset
 		 < offset in a file >
 Output		:void
 Return		:void*
 		 < candidate memory address >
 Description	:seach memory area as a candidate for mmap
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* search_mmap_candidate(struct process *proc, size_t length,
					int prot, int flags, int fd, off_t offset)
{
	struct list *pos;
	struct vm *vm;
	struct vm *vm_next;
	struct vm *vm_prev;
	struct vm candidate;
	
	if (UNLIKELY(is_empty_list(&proc->mspace->list_vm))) {
		return((void*)MMAP_START);
	}
	
	candidate.start = MMAP_START + offset;
	candidate.end = candidate.start + length;
	
	if (proc->mspace->start_stack <= candidate.end) {
		return(MAP_FAILED);
	}
	
	list_for_each(pos, &proc->mspace->list_vm) {
		vm = get_entry(pos, struct vm, link_vm);
		if (vm->start < MMAP_START) {
			continue;
		}
		
		if (UNLIKELY(pos->next == &proc->mspace->list_vm)) {
			if (vm->end < candidate.start) {
				return((void*)candidate.start);
			}
		}
		
		if (UNLIKELY(pos->prev == &proc->mspace->list_vm)) {
			if ((candidate.start <= vm->start) &&
				(candidate.end <= vm->start)) {
				return((void*)candidate.start);
			}
		}
		vm_next = get_entry(pos->next, struct vm, link_vm);
		vm_prev = get_entry(pos->prev, struct vm, link_vm);
		
		if ((vm_prev->end <= candidate.start) &&
				(candidate.end <= vm->start))
		{
			return((void*)candidate.start);
		}
		
		if ((vm->end <= candidate.start) &&
			(candidate.end <= vm_next->start)) {
			return((void*)candidate.start);
		}
		
		candidate.start = (unsigned long)
				PageAlignU((const void*)(vm->end + 1));
		candidate.end = candidate.start + length;
		
		if (proc->mspace->start_stack <= candidate.end) {
			return(MAP_FAILED);
		}
	}
	return(MAP_FAILED);
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
 Funtion	:_free_vm
 Input		:struct process *proc
 		 < process which has vm areas >
 Output		:void
 Return		:void
 Description	:free virtual memory for a user process
==================================================================================
*/
LOCAL INLINE void _free_vm(struct process *proc)
{
	struct vm *vm;
	struct vm *temp;
	
	if (is_empty_list(&proc->mspace->list_vm)) {
		return;
	}
	/* -------------------------------------------------------------------- */
	/* free all virtual memory						*/
	/* -------------------------------------------------------------------- */
	list_for_each_entry_safe(vm, temp, &proc->mspace->list_vm, link_vm) {
		if (vm->nr_pages && vm->pages) {
			free_vm_pages(vm);
		}
		__free_vm(vm);
	}
}

/*
==================================================================================
 Funtion	:__free_vm
 Input		:struct vm *vm
 		 < vm to free >
 Output		:void
 Return		:void
 Description	:free a vm struct
==================================================================================
*/
LOCAL INLINE void __free_vm(struct vm *vm)
{
	del_list(&vm->link_vm);
	kmem_cache_free(vm_cache, (void*)vm);
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
