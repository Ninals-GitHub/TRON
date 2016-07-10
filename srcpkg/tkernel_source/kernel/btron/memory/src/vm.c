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
#include <bk/memory/slab.h>
#include <bk/memory/page_fault.h>
#include <bk/memory/prot.h>
#include <bk/fs/vfs.h>
#include <bk/uapi/berrno.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL INLINE void _free_vm(struct process *proc);
LOCAL INLINE void __free_vm(struct vm *vm);
LOCAL int insert_vm_list(struct vm *new, struct process *proc);
LOCAL void copy_vm_contents(struct vm *to, struct vm *from);
LOCAL int verify_vm_contents(struct vm *dest, struct vm *with);
LOCAL int vm_file_map(struct vm *vm, unsigned long addr);

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
	
	if (UNLIKELY(!mspace_cache)) {
		vd_printf("error:mspace_cache\n");
		return(-ENOMEM);
	}

	vm_cache = kmem_cache_create(vm_cache_name, sizeof(struct vm), 0, 0, NULL);
	
	if (UNLIKELY(!vm_cache)) {
		vd_printf("error:vm_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_mm
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy virtual memory management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_mm(void)
{
	kmem_cache_destroy(mspace_cache);
	kmem_cache_destroy(vm_cache);
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
	
	if (UNLIKELY(!mspace)) {
		return(mspace);
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
EXPORT struct vm* alloc_vm(void)
{
	struct vm *vm = (struct vm*)kmem_cache_alloc(vm_cache, 0);
	
	if (UNLIKELY(!vm)) {
		return(vm);
	}
	
	memset((void*)vm, 0x00, sizeof(struct vm));
	
	init_list(&vm->link_vm);
	
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
	//printf("%s\n", __func__);
	/* -------------------------------------------------------------------- */
	/* free virtual memory for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_vm(proc);
	
	/* -------------------------------------------------------------------- */
	/* free all user page tables and page directory				*/
	/* -------------------------------------------------------------------- */
	free_user_pagetables(proc->mspace->pde);
	
	//free_pagedir(proc->mspace->pde);
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
	//printf("%s\n", __func__);
	//show_vm_list(proc);
	/* -------------------------------------------------------------------- */
	/* free user space page tables						*/
	/* -------------------------------------------------------------------- */
	free_user_pagetables(proc->mspace->pde);
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
	
	vp = (struct page**)kcalloc(nr_pages, sizeof(struct page*), 0);
	
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
	int i;
	
	for (i = 0;i < vm->nr_pages;i++) {
		if (vm->pages[i]) {
			free_page(vm->pages[i]);
		}
	}
	
	vm->nr_pages = 0;
	
	kfree((void*)vm->pages);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_address_vm
 Input		:struct process *proc
 		 < get vm from the process >
 		 unsigned long start
 		 < user start address which belongs to vm of process >
 		 unsigned long end
 		 < user end address which belongs to vm of process >
 Output		:void
 Return		:struct vm*
 		 < vm to which address belongs >
 Description	:get vm to which address belongs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vm*
get_address_vm(struct process *proc, unsigned long start, unsigned long end)
{
	struct vm *vm;
	
	list_for_each_entry(vm, &proc->mspace->list_vm, link_vm) {
		if (vm->start <= start && end < vm->end) {
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
	pde_t *new_pde;
	struct memory_space *ms_to = to->mspace;
	struct memory_space *ms_from = from->mspace;
	
	if (UNLIKELY(!to->mspace)) {
		printf("copy_mm:error:unexpected memory space\n");
		return(-ENOMEM);
	}
	
	/* -------------------------------------------------------------------- */
	/* update memory space information					*/
	/* -------------------------------------------------------------------- */
	ms_to->process_size = ms_from->process_size;
	ms_to->count = 1;
	ms_to->shared_vm = ms_from->shared_vm;
	ms_to->exec_vm = ms_from->exec_vm;
	ms_to->stack_vm = ms_from->stack_vm;
	ms_to->start_code = ms_from->start_code;
	ms_to->end_code = ms_from->end_code;
	ms_to->start_data = ms_from->start_data;
	ms_to->end_data = ms_from->end_data;
	ms_to->start_brk = ms_from->start_brk;
	ms_to->end_brk = ms_from->end_brk;
	ms_to->start_stack = ms_from->start_stack;
	ms_to->end_stack = ms_from->end_stack;
	ms_to->start_arg = ms_from->end_arg;
	ms_to->end_arg = ms_from->end_arg;
	ms_to->start_env = ms_from->end_env;
	ms_to->exe_fd = ms_from->exe_fd;
	
	/* -------------------------------------------------------------------- */
	/* allocate page directory						*/
	/* -------------------------------------------------------------------- */
	new_pde = alloc_pagedir();
	
	if (UNLIKELY(!new_pde)) {
		err = -ENOMEM;
		printf("copy_mm:err_alloc_pagedir\n");
		goto err_alloc_pagedir;
	}
	
	to->mspace->pde = new_pde;
#if 0	// copy at switch_ms
	/* -------------------------------------------------------------------- */
	/* copy kernel page tables						*/
	/* -------------------------------------------------------------------- */
	err = copy_kernel_pagetables(from, to);
	
	if (UNLIKELY(err)) {
		printf("copy_mm:err_copy_kernel_pagetalbes\n");
		goto err_copy_kernel_pagetalbes;
	}
#endif

	/* -------------------------------------------------------------------- */
	/* copy user page tables						*/
	/* -------------------------------------------------------------------- */
	err = copy_user_pagetable(from, to, 0, PROCESS_SIZE, PAGE_COW_COPY);
	
	if (UNLIKELY(err)) {
		printf("copy_mm:copy_vm_failed\n");
		goto copy_vm_failed;
	}

	/* -------------------------------------------------------------------- */
	/* copy user vm								*/
	/* -------------------------------------------------------------------- */
	err = copy_vm(from, to);
	
	if (UNLIKELY(err)) {
		printf("copy_mm:copy_vm_failed\n");
		goto copy_vm_failed;
	}
	
	return(0);

copy_vm_failed:
	printf("unexpected error at %s [free_pagedir_tables_all]\n", __func__);
	free_pagedir_tables_all(to->mspace->pde);
	free_memory_space(to->mspace);
	return(err);

err_alloc_pagedir:
	return(err);
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
	struct vm *vm_to;
	
	int err;
	
	/* -------------------------------------------------------------------- */
	/* copy other vms							*/
	/* -------------------------------------------------------------------- */
	list_for_each_entry(vm_from, &from->mspace->list_vm, link_vm) {
		
		vm_to = alloc_vm();
		
		if (UNLIKELY(!vm_to)) {
			err = -ENOMEM;
			goto failed_alloc_vm;
		}
		
		vm_to->start = vm_from->start;
		vm_to->end = vm_from->end;
		vm_to->prot = vm_from->prot;
		vm_to->mmap_flags = vm_from->mmap_flags;
		vm_to->nr_pages = vm_from->nr_pages;
		vm_to->mspace = to->mspace;
		vm_to->vnode = vm_from->vnode;
		vm_to->offset = vm_from->offset;
		vm_to->length = vm_from->length;
		
		err = copy_vm_pages(vm_from, vm_to);
		
		if (UNLIKELY(err)) {
			goto failed_copy_vm_pages;
		}
		
		err = insert_vm_list(vm_to, to);
		
		if (UNLIKELY(err)) {
			goto failed_insert_vm;
		}
	}
	
	//show_vm_list(to);
	//show_vm_list(from);
	
	return(0);

failed_insert_vm:
failed_copy_vm_pages:
failed_alloc_vm:
	free_vm_all(to);
	return(err);
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
	
	if (UNLIKELY(!pages)) {
		return(-ENOMEM);
	}
	
	for (i = 0;i < vm_from->nr_pages;i++) {
		if ((pages)[i]) {
			printf("unexpected error at %s pages[%d]:0x%08X\n", __func__, i, pages[i]);
			for(;;);
		}
		if (vm_from->pages[i] && vm_from->pages[i] < (struct page*)KERNEL_BASE_ADDR) {
			printf("unexpected error at %s vm_from->pages[%d]:0x%08X\n", __func__, i, vm_from->pages[i]);
			printf("start:0x%08X end:0x%08X\n", vm_from->start, vm_from->end);
			for(;;);
		}
		
		(pages)[i] = (vm_from->pages)[i];
		if (pages[i]) {
			(vm_from->pages)[i]->count++;
			if (vm_from->prot & PROT_WRITE) {
				pages[i]->flags |= PAGE_COW;
			}
		}
	}
	
	vm_to->pages = pages;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:map_vm
 Input		:struct process *proc
 		 < a process to create its vm >
 		 unsigned long mmap_start
 		 < start address of user space >
 		 unsigned long mmap_end
 		 < end address of user space >
 		 size_t lenght
 		 < length of a memory map >
 		 unsigned int prot
 		 < permission >
 		 unsigned int mmap_flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor to map >
 		 off_t offset
 		 < offset in a file >
 Output		:void
 Return		:int
 		 < result >
 Description	:map a virtual memory for user space
 		 this function is currently used for test only
 		 future work;
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int map_vm(struct process *proc,
			unsigned long mmap_start, unsigned long mmap_end,
			size_t length, unsigned int prot,
			unsigned int mmap_flags, int fd, loff_t offset)
{
	int err;
	struct vm *vm = alloc_vm();
	struct file *filp;
	
	if (UNLIKELY(!vm)) {
		printf("map_vm failed:1\n");
		return(-ENOMEM);
	}
	
	if (UNLIKELY(mmap_end <= mmap_start)) {
		printf("map_vm failed:2\n");
		err = -EINVAL;
		goto failed_invalid_space;
	}
	
	vm->start = mmap_start & PAGE_MASK;
	vm->end = RoundPage(mmap_end);
	vm->nr_pages = PageCount(mmap_end - mmap_start);
	vm->length = length;
	
	if (0 <= fd) {
		filp = get_open_file(fd);
		
		if (UNLIKELY(!filp)) {
			printf("map_vm failed:get_open_file\n");
			err = -EACCES;
			goto failed_open_file;
		}
		
		vm->vnode = filp->f_vnode;
	} else {
		vm->vnode = NULL;
	}
	vm->offset = offset;
	vm->pages = alloc_vm_pages(vm->nr_pages);
	
	if (UNLIKELY(!vm->pages)) {
		err = -ENOMEM;
		printf("map_vm failed:3\n");
		goto failed_alloc_vm_pages;
	}
	
	vm->prot = prot;
	vm->mmap_flags = mmap_flags;
	vm->mspace = proc->mspace;
	
	if (mmap_flags & MAP_SHARED) {
		panic("not implemented MAP_SHARED at %s\n", __func__);
	}
	
	err = map_pages_to_vm(proc, vm);
	
	if (UNLIKELY(err)) {
		printf("map_vm failed:5\n");
		goto failed_map_pages_to_vm;
	}
	
	err = insert_vm_list(vm, proc);
	
	if (UNLIKELY(err)) {
		printf("map_vm failed:6\n");
		err = -EINVAL;
		goto failed_insert_vm;
	}
	
	return(0);
	
failed_insert_vm:
failed_map_pages_to_vm:
	_free_pagetables(proc->mspace->pde, mmap_start, mmap_end);
	free_vm_pages(vm);
//	printf("__free_vm[1:%s]\n",__func__);
	__free_vm(vm);
	return(err);

failed_open_file:
failed_invalid_space:
failed_alloc_vm_pages:
//	printf("__free_vm[2:%s]\n",__func__);
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
	
	if (UNLIKELY(is_empty_list(&mspace->list_vm))) {
		return(err);
	}
	
	if (UNLIKELY(mmap_end <= mmap_start)) {
		
		return(-ENOMEM);
	}
	
	//printf("mmap_start:0x%08X ", mmap_start);
	//printf("mmap_end:0x%08X\n", mmap_end);
	
	prev = next = NULL;
	
#if 1
	list_for_each_entry_safe(vm, temp, &proc->mspace->list_vm, link_vm) {
		//printf("vm->start:0x%08X ", vm->start);
		//printf("vm->end:0x%08X\n", vm->end);
		if (vm->start == mmap_start && mmap_end == vm->end) {
			_free_pagetables(proc->mspace->pde, vm->start, vm->end);
			
			del_list(&vm->link_vm);
			
			if (vm->nr_pages && vm->pages) {
				free_vm_pages(vm);
			}
			//printf("__free_vm[1:%s]\n",__func__);
			__free_vm(vm);
			return(err);
		} else if (vm->start <= mmap_start && mmap_end <= vm->end) {
		/* ------------------------------------------------------------ */
		/* split vm area						*/
		/* ------------------------------------------------------------ */
			unsigned long nr_prev = 0;
			unsigned long nr_next = 0;
			unsigned long nr_del;
			unsigned long i;
			unsigned long index;
			
#if 0
			printf("unmap:vm->start[0x%08X] ", vm->start);
			printf("unmap:vm->end[0x%08X]\n", vm->end);
			printf("unmap:mmap_start[0x%08X] ", mmap_start);
			printf("unmap:mmap_end[0x%08X]\n", mmap_end);
#endif
			
			/* ---------------------------------------------------- */
			/* make prev						*/
			/* ---------------------------------------------------- */
			if (vm->start != mmap_start) {
				prev = alloc_vm();
				
				if (UNLIKELY(!prev)) {
					return(-ENOMEM);
				}
				
				prev->start = vm->start;
				prev->end = mmap_start;
				nr_prev = PageCount(prev->end - prev->start);
				
				prev->pages = alloc_vm_pages(nr_prev);
				
				if (UNLIKELY(!prev->pages)) {
					err = -ENOMEM;
					goto failed_prev;
				}
				
				prev->prot = vm->prot;
				prev->mspace = vm->mspace;
				prev->vnode = vm->vnode;
				if (prev->vnode) {
					prev->offset = vm->offset;
				}
				prev->length = prev->end - prev->start;
				if (UNLIKELY(vm->length < prev->length)) {
					prev->length = vm->length;
				}
			} else {
				prev = NULL;
			}
			
			if (vm->end != mmap_end) {
				next = alloc_vm();
				
				if (UNLIKELY(!next)) {
					err = -ENOMEM;
					goto failed_alloc_vm_next;
				}
				
				next->start = mmap_end;
				next->end = vm->end;
				nr_next = PageCount(next->end - next->start);
				
				next->pages = alloc_vm_pages(nr_next);
				
				if (UNLIKELY(!next->pages)) {
					err = -ENOMEM;
					goto failed_next;
				}
				
				next->prot = vm->prot;
				next->mspace = vm->mspace;
				next->vnode = vm->vnode;
				if (next->vnode) {
					next->offset = next->start - vm->start + vm->offset;
				}
				next->length = next->end - next->start;
				if (UNLIKELY(vm->length < next->length)) {
					next->length = vm->length;
				}
			} else {
				next = NULL;
			}
			
			if (prev && next) {
				nr_del = PageCount(next->start - prev->end);
			} else if (prev && !next) {
				nr_del = PageCount(vm->end - prev->end);
			} else /* if(!prev && next) */{
				nr_del = PageCount(next->end - next->start);
			}
			
			/* ---------------------------------------------------- */
			/* copy pages to prev area				*/
			/* ---------------------------------------------------- */
			if (prev) {
				for (i = 0;i < nr_prev;i++) {
					prev->pages[i] = vm->pages[i];
				}
				
				if (vm->nr_pages < i) {
					printf("unexpected error at %s[1] i:%d vm->nr_pages:%d\n", i, vm->nr_pages);
				}
			} else {
				i = 0;
			}
			
			index = i;
			/* ---------------------------------------------------- */
			/* free pages of split area				*/
			/* ---------------------------------------------------- */
			_free_pagetables(proc->mspace->pde, mmap_start, mmap_end);
			
			/* ---------------------------------------------------- */
			/* copy pages to next area				*/
			/* ---------------------------------------------------- */
			if (next) {
				for (i = index + nr_del;i < (index + nr_del + nr_next);i++) {
					next->pages[i - (index + nr_del)] = vm->pages[i];
				}
				
				if (vm->nr_pages < i) {
					printf("unexpected error at %s[2] i:%d vm->nr_pages:%d\n", i, vm->nr_pages);
				}

			}
			
			if (prev) {
				add_list_element(&prev->link_vm,
						vm->link_vm.prev, &vm->link_vm);
			}
			
			if (next) {
				add_list(&next->link_vm, &vm->link_vm);
			}
			
#if 0
			if (prev) {
				printf("prev->start:0x%08X prev->end:0x%08X\n", prev->start, prev->end);
			}
			printf("vm->start:0x%08X vm->end:0x%08X\n", vm->start, vm->end);
			if (next) {
				printf("next->start:0x%08X next->end:0x%08X\n", next->start, next->end);
			}
#endif
			
			del_list(&vm->link_vm);
			
			if (vm->nr_pages && vm->pages) {
				free_vm_pages(vm);
			}
			
			__free_vm(vm);
		}
	}
#endif

	
	return(err);
	
failed_next:
	free_vm_pages(prev);
failed_prev:
//	printf("__free_vm[3:%s]\n",__func__);
	__free_vm(next);
failed_alloc_vm_next:
//	printf("__free_vm[4:%s]\n",__func__);
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
	
	//printf("candidate:start=0x%08X, end=0x%08X\n", candidate.start, candidate.end);
	
	if (UNLIKELY(proc->mspace->start_stack <= candidate.end)) {
		printf("start_stack[0x%08X] <= candidate.end[0x%08X]\n", proc->mspace->start_stack, candidate.end);
		return(MAP_FAILED);
	}
	
	list_for_each(pos, &proc->mspace->list_vm) {
		vm = get_entry(pos, struct vm, link_vm);
		if (vm->start < MMAP_START) {
			continue;
		}
		
		//printf("vm:start=0x%08X, end=0x%08X\n", vm->start, vm->end);
		
		if (UNLIKELY(pos->next == &proc->mspace->list_vm)) {
			if (vm->end < candidate.start) {
				return((void*)candidate.start);
			}
			
			vm_next = NULL;
		} else {
			vm_next = get_entry(pos->next, struct vm, link_vm);
		}
		
		/* ------------------------------------------------------------ */
		/* vm_prev							*/
		/* ------------------------------------------------------------ */
		if (UNLIKELY(pos->prev == &proc->mspace->list_vm)) {
			if ((candidate.start <= vm->start) &&
				(candidate.end <= vm->start)) {
				return((void*)candidate.start);
			} else {
				vm_next = NULL;
			}
		} else {
			vm_prev = get_entry(pos->prev, struct vm, link_vm);
			
			if ((vm_prev->end <= candidate.start) &&
					(candidate.end <= vm->start))
			{
				return((void*)candidate.start);
			}
		}
		
		/* ------------------------------------------------------------ */
		/* vm_next							*/
		/* ------------------------------------------------------------ */
		if (vm_next) {
			if ((vm->end <= candidate.start) &&
				(candidate.end <= vm_next->start)) {
				return((void*)candidate.start);
			}
		} else {
			if (vm->end <= candidate.start) {
				return((void*)candidate.start);
			} else {
				//printf("candidate.start=0x%08X vm->end=0x%08X\n", candidate.start, vm->end);
				candidate.start = (unsigned long)
					PageAlignU((const void*)(vm->end));
				if (proc->mspace->start_stack <= candidate.end) {
					return(MAP_FAILED);
				}
				return((void*)candidate.start);
			}
		}
		
		
		/* ------------------------------------------------------------ */
		/* go to next vm						*/
		/* ------------------------------------------------------------ */
		candidate.start = (unsigned long)
				PageAlignU((const void*)(vm->end));
		candidate.end = candidate.start + length;
		
		if (proc->mspace->start_stack <= candidate.end) {
			return(MAP_FAILED);
		}
	}
	return(MAP_FAILED);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:switch_ms
 Input		:struct process *from
 		 < switch from >
 		 struct process *to
 		 < switch to >
 Output		:void
 Return		:void
 Description	:switch memory space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void switch_ms(struct process *from, struct process *to)
{
	int err;
	
	err = copy_kernel_pagetables(from, to);
	
	if (UNLIKELY(err)) {
		printf("switch_ms:unexpected error\n");
		for(;;);
	}
	
	//show_pagetables(to, to->mspace->start_code, to->mspace->start_code + 0x4000);
	
	loadPdbr((unsigned long)to->mspace->pde - KERNEL_BASE_ADDR);
	
	if (UNLIKELY(!from->state)) {
		free_pagetable((pte_t*)from->mspace->pde);
	}
	
	ASM (
	"jmp	flush_tlb_after_load_pdbr	\n\t"
	"flush_tlb_after_load_pdbr:"
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_extend_brk
 Input		:struct process *proc
 		 < process to extend its brk >
 		 unsigned long new_brk
 		 < extension size of new break. must be aligned to page size >
 Output		:void
 Return		:void*
 		 < new break >
 Description	:extend break
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* vm_extend_brk(struct process *proc, unsigned long new_brk)
{
	struct memory_space *mspace = proc->mspace;
	struct vm *vm_brk;
	struct vm vm_extend;
	void *addr;
	unsigned long nr_ex_pages;
	struct page **new_pages;
	int i;
	int err;
	
	if (UNLIKELY(mspace->start_brk == mspace->end_brk)) {
		addr = xmmap(proc, (void*)mspace->start_brk, new_brk,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		
		if (UNLIKELY((void*)MAP_FAILED == addr)) {
			return(addr);
		}
		
		goto new_brk_out;
	}
	
	vm_brk = get_address_vm(proc, mspace->start_brk, mspace->start_brk);
	
	//printf("vm_brk->start:0x%08X ", vm_brk->start);
	//printf("vm_brk->end:0x%08X\n", vm_brk->end);
	
	if (!vm_brk) {
		return((void*)MAP_FAILED);
	}
	
	nr_ex_pages = PageCount(new_brk);
	
	new_pages = alloc_vm_pages(vm_brk->nr_pages + nr_ex_pages);
	
	if (!new_pages) {
		return((void*)MAP_FAILED);
	}
	
	for (i = 0;i < vm_brk->nr_pages;i++) {
		new_pages[i] = vm_brk->pages[i];
	}
	
	for (i = vm_brk->nr_pages;i < vm_brk->nr_pages + nr_ex_pages;i++) {
		new_pages[i] = alloc_zeroed_page();
	}
	
	vm_extend.nr_pages = nr_ex_pages;
	vm_extend.pages = &new_pages[vm_brk->nr_pages];
	vm_extend.prot = vm_brk->prot;
	vm_extend.mspace = vm_brk->mspace;
	vm_extend.start = vm_brk->end;
	vm_extend.end = vm_extend.start + new_brk;
	
	err = map_pages_to_vm(proc, &vm_extend);
	
	if (err) {
		vm_extend.pages = new_pages;
		free_vm_pages(&vm_extend);
		return((void*)MAP_FAILED);
	}
	
	nr_ex_pages += vm_brk->nr_pages;
	free_vm_pages(vm_brk);
	
	vm_brk->pages = new_pages;
	vm_brk->end += new_brk;
	vm_brk->nr_pages = nr_ex_pages;
	
	
new_brk_out:
	mspace->end_brk += new_brk;
	//show_vm_list(proc);
	
	return((void*)mspace->end_brk);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_extend_stack
 Input		:struct process *proc
 		 < process to extend its stack area >
 		 unsigned long new_extend
 		 < new extension size >
 Output		:void
 Return		:int
 		 < result >
 Description	:extend stack
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vm_extend_stack(struct process *proc, unsigned long new_extend)
{

	struct memory_space *mspace = proc->mspace;
	struct vm *vm_stack;
	struct vm vm_extend;
	unsigned long nr_ex_pages;
	struct page **new_pages;
	int i;
	int err;
	
	vm_stack = get_address_vm(proc, mspace->start_stack, mspace->start_stack);
	
	printf("vm_stack->start:0x%08X ", vm_stack->start);
	printf("vm_statck->end:0x%08X\n", vm_stack->end);
	
	if (!vm_stack) {
		return(-ENOMEM);
	}
	
	nr_ex_pages = PageCount(new_extend);
	
	new_pages = alloc_vm_pages(vm_stack->nr_pages + nr_ex_pages);
	
	if (!new_pages) {
		return(-ENOMEM);
	}
	
	for (i = 0;i < vm_stack->nr_pages;i++) {
		new_pages[i] = vm_stack->pages[i];
	}
	
	for (i = vm_stack->nr_pages;i < vm_stack->nr_pages + nr_ex_pages;i++) {
		new_pages[i] = alloc_zeroed_page();
	}
	
	vm_extend.nr_pages = nr_ex_pages;
	vm_extend.pages = &new_pages[vm_stack->nr_pages];
	vm_extend.prot = vm_stack->prot;
	vm_extend.mspace = vm_stack->mspace;
	vm_extend.start = vm_stack->start - new_extend;
	vm_extend.end = vm_stack->start;
	
	err = map_pages_to_vm(proc, &vm_extend);
	
	if (err) {
		vm_extend.pages = new_pages;
		free_vm_pages(&vm_extend);
		return(err);
	}
	
	nr_ex_pages += vm_stack->nr_pages;
	free_vm_pages(vm_stack);
	
	vm_stack->pages = new_pages;
	vm_stack->start -= new_extend;
	vm_stack->nr_pages = nr_ex_pages;
	
	mspace->start_stack = vm_stack->start;
	vd_printf("new vm_stack->start:0x%08X ", vm_stack->start);
	vd_printf("new vm_statck->end:0x%08X\n", vm_stack->end);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_initial_stack
 Input		:struct process *proc
 		 < process to map its initial stack >
 		 int nr_pages
 		 < number of pages for stack >
 		 struct page **pages
 		 < pages of initial stack >
 Output		:void
 Return		:int
 		 < result >
 Description	:map initial stack page to vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vm_initial_stack(struct process *proc, int nr_pages, struct page **pages)
{
	struct vm *vm = alloc_vm();
	int err;
	int nr;
	
	if (UNLIKELY(!vm)) {
		return(-ENOMEM);
	}
	
	vm->start = PAGE_ALIGN(PROC_STACK_START);
	vm->end = vm->start + PROC_STACK_SIZE;
	vm->nr_pages = nr_pages;
	vm->vnode = NULL;
	vm->offset = 0;
	vm->pages = alloc_vm_pages(vm->nr_pages);
	
	if (UNLIKELY(!vm->pages)) {
		err = -ENOMEM;
		printf("map_vm failed:3\n");
		goto failed_alloc_vm_pages;
	}
	
	//printf("stack vm:");
	for (nr = 0;nr < vm->nr_pages;nr++) {
		vm->pages[nr] = pages[nr];
	//	printf("0x%08X " ,page_to_paddr(pages[nr]));
	}
	//printf("\n");
	
	vm->prot = PROT_READ | PROT_WRITE | PROT_GROWSUP;
	vm->mmap_flags = MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS;
	vm->mspace = proc->mspace;
	
	err = map_pages_to_vm(proc, vm);
	
	if (UNLIKELY(err)) {
		err = -ENOMEM;
		goto failed_map_pages_to_vm;
	}
	
	err = insert_vm_list(vm, proc);
	
	if (UNLIKELY(err)) {
		err = -EINVAL;
		goto failed_insert_vm;
	}
	
	//flush_tlb();
	
	//printf("initial stack is made!!!!!1\n");
	
	return(0);
	
failed_insert_vm:
failed_map_pages_to_vm:
	_free_pagetables(proc->mspace->pde, vm->start, vm->end);
	free_vm_pages(vm);
failed_alloc_vm_pages:
	__free_vm(vm);
	printf("failed %s\n", __func__);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:activate_vm_page
 Input		:struct process *proc
 		 < process to activate is page >
 		 struct vm *vm
 		 < virtual memory to activate its page >
 		 unsigned int la
 		 < virtual address which vm includes >
 		 unsigned int error_code
 		 < error code of a page fault >
 Output		:void
 Return		:int
 		 < result >
 Description	:activate a paged whic is already mapped to vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int activate_vm_page(struct process *proc, struct vm *vm,
				unsigned long la, unsigned int error_code)
{
	pde_t *pde = proc->mspace->pde;
	unsigned long index;
	struct page *page;
	int err;
	
	index = (la - vm->start) / PAGESIZE;
	
	/* -------------------------------------------------------------------- */
	/* activate memory map of a file					*/
	/* -------------------------------------------------------------------- */
	if (vm->vnode && !vm->pages[index]) {
		int findex = vm_file_map(vm, la);
		
		if (UNLIKELY(findex < 0)) {
			panic("0:unexpected error at %s[0x%08X]\n", __func__, la);
			return(findex);
		}
		page = vm->pages[findex];
		
		if (vm->pages[index]->flags & PAGE_COW) {
			//printf("COW!!!!!![2]0x%08X 0x%08X\n", vm->vnode, la);
			vm->pages[index]->flags &= ~PAGE_COW;
		}
		goto activate_page;
	}
	
	/* -------------------------------------------------------------------- */
	/* activate a non-exist page						*/
	/* -------------------------------------------------------------------- */
	if (!vm->pages[index]) {
		page = alloc_zeroed_page();
		if (UNLIKELY(!page)) {
			panic("1-3:unexpected error at %s[0x%08X]\n", __func__, la);
			return(-EFAULT);
		}
		
		vm->pages[index] = page;
		//printf("COW!!!!!![3]0x%08X 0x%08X\n", page_to_address(page), la);
		
		goto activate_page;
	}
	
	/* -------------------------------------------------------------------- */
	/* activate a cow page							*/
	/* -------------------------------------------------------------------- */
	if ((vm->pages[index]->flags & PAGE_COW) &&
			(vm->pages[index]->count != 1)) {
		page = alloc_page();
		
		if (UNLIKELY(!page)) {
			panic("1-1:unexpected error at %s[0x%08X]\n", __func__, la);
			return(-EFAULT);
		}
		
		if (UNLIKELY(!vm->pages[index])) {
			panic("1-2::unexpected error at %s[0x%08X]\n", __func__, la);
		}
		
		copy_page_contents(page, vm->pages[index]);
		
		page->flags = vm->pages[index]->flags & ~PAGE_COW;
		
		free_page(vm->pages[index]);
		
		vm->pages[index] = page;
	}
	
	if (vm->pages[index]->flags & PAGE_COW) {
		//printf("COW!!!!!![2]0x%08X 0x%08X\n", vm->vnode, la);
		vm->pages[index]->flags &= ~PAGE_COW;
		//return(0);
	}
	
	page = vm->pages[index];
	
activate_page:
	if (UNLIKELY(!page && !vm->pages[index])) {
		panic("2:unexpected error at %s[index=%d, addr=0x%08X]\n", __func__, index, la);
	}
	
	if (vm->vnode) {
		if (!vm->pages[index]->count)
		printf("acti[la:0x%08X vm->pages[index]->count:%d\n", la, vm->pages[index]->count);
	}
	
	err = activate_page(pde, page, vm, la);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_check_access
 Input		:void *user_addr
 		 < user space address >
 		 size_t size
 		 < memory size to check >
 		 int prot
 		 < memory protection >
 Output		:void
 Return		:int
 		 < result >
 Description	:check user memory permission
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vm_check_access(void *user_addr, size_t size, int prot)
{
	unsigned long addr = (unsigned long)user_addr;
	struct vm *vm;
	int rng;
	
	rng = (get_current_task()->tskatr & TA_RNG3) >> 8;
	
	if (UNLIKELY(!rng)) {
		return(0);
	}
	
	if (PROCESS_SIZE < addr) {
		//for(;;);
		return(-EACCES);
	}
	
	vm = get_address_vm(get_current(), addr, addr + size);
	
	if (UNLIKELY(!vm)) {
		return(-EACCES);
	}
	
	if ((vm->prot & prot) == prot) {
		return(0);
	}
	
	return(-EACCES);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_mprotect
 Input		:void *addr
 		 < start address of a memory region to set memory protectoin >
 		 size_t len
 		 < size of a memory region >
 		 int prot
 		 < protection >
 Output		:void
 Return		:int
 		 < result >
 Description	:set protection on a region of memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vm_mprotect(void *addr, size_t len, int prot)
{
	struct vm *vm;
	
	vm = get_address_vm(get_current(), (unsigned long)addr, (size_t)addr + len);
	
	if (UNLIKELY(!vm)) {
		return(-ENOMEM);
	}
	
#if 0
	printf("prot=0x%02X\n", vm->prot);
#endif
	vm->prot |= prot;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_vm_list
 Input		:struct process *proc
 		 < show its vms >
 Output		:void
 Return		:void
 Description	:show vm list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void show_vm_list(struct process *proc)
{
	struct memory_space *mspace = proc->mspace;
	struct vm *vm;
	struct vm *temp;
	int index = 0;
	
	printf("show vm list[pid=%d]:\n", proc->pid);
	
	if (UNLIKELY(is_empty_list(&mspace->list_vm))) {
		printf("there is no vm list\n");
		return;
	}
	
#if 0
	printf("start_code =0x%08X, end_code =0x%08X\n",
		mspace->start_code, mspace->end_code);
	printf("start_data =0x%08X, end_data =0x%08X\n",
		mspace->start_data, mspace->end_data);
	printf("start_brk  =0x%08X, end_brk  =0x%08X\n",
		mspace->start_brk, mspace->end_brk);
	printf("start_stack=0x%08X, end_stack=0x%08X\n",
		mspace->start_stack, mspace->end_stack);
#endif
	list_for_each_entry_safe(vm, temp, &proc->mspace->list_vm, link_vm) {
		//if (0x40000000 <= vm->start)
		printf("[%d]:vm->start=0x%08X, vm->end=0x%08X 0x%08X 0x%04X\n",
				index++, vm->start, vm->end, vm->vnode, vm->prot);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_vm_cow
 Input		:unsigned long addr
 		 < address to check whether its page is cow >
 Output		:void
 Return		:int
 		 < result >
 Description	:check whether address is in cow page
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int is_vm_cow(unsigned long addr)
{
	struct vm *vm;
	int index;
	
	vm = get_address_vm(get_current(), addr, addr);
	
	if (UNLIKELY(!vm)) {
		return(0);
	}
	
	index = (addr - vm->start) / PAGESIZE;
	
	return((vm->pages[index]->flags & PAGE_COW) == PAGE_COW);
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
	
	if (UNLIKELY(is_empty_list(&proc->mspace->list_vm))) {
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
//	printf("__free_vm!!!!!!!!!!!!!\n");
	del_list(&vm->link_vm);
	kmem_cache_free(vm_cache, (void*)vm);
}


/*
==================================================================================
 Funtion	:insert_vm_list
 Input		:struct vm *new
 		 < an insertee >
 		 struct process *proc
 		 < a process to be inserted a vm >
 Output		:void
 Return		:int
 		 < result >
 Description	:insert vm to a vm space of a process
==================================================================================
*/
LOCAL int insert_vm_list(struct vm *new, struct process *proc)
{
	struct list *pos;
	struct vm *proc_vm;
	struct vm *proc_vm_next;
	struct vm *proc_vm_prev;
	
	if (UNLIKELY(is_empty_list(&proc->mspace->list_vm))) {
		add_list(&new->link_vm, &proc->mspace->list_vm);
		return(0);
	}
	
	list_for_each(pos, &proc->mspace->list_vm) {
		proc_vm = get_entry(pos, struct vm, link_vm);
		
		if (UNLIKELY(pos->next == &proc->mspace->list_vm)) {
			if (new->end <= proc_vm->start) {
				add_list(&new->link_vm, pos->prev);
			} else {
				add_list(&new->link_vm, pos);
			}
			return(0);
		}
		
		proc_vm_next = get_entry(pos->next, struct vm, link_vm);
		
		if (UNLIKELY(pos->prev == &proc->mspace->list_vm)) {
			if (new->end <= proc_vm->start) {
				add_list(&new->link_vm, pos->prev);
				return(0);
			}
			
			continue;
		}
		
		proc_vm_prev = get_entry(pos->prev, struct vm, link_vm);
		
		if ((proc_vm_prev->end <= new->start) &&
				(new->end <= proc_vm->start)) {
			add_list(&new->link_vm, pos->prev);
			return(0);
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
==================================================================================
 Funtion	:copy_vm_contents
 Input		:struct vm *to
 		 < copy to >
 		 struct vm *from
 		 < copy from >
 Output		:void
 Return		:void
 Description	:copy contents of vm
==================================================================================
*/
LOCAL void copy_vm_contents(struct vm *to, struct vm *from)
{
	int i;
	
	//printf("from->nr_pages:%d ", from->nr_pages);
	//printf("to->nr_pages:%d\n", to->nr_pages);
	
	for (i = 0;i < from->nr_pages;i++) {
		struct page *page_from = from->pages[i];
		struct page *page_to = to->pages[i];
		
		unsigned long addr_from = page_to_address(page_from);
		unsigned long addr_to = page_to_address(page_to);
		
		//printf("addr_from:0x%08X ", addr_from);
		//printf("addr_to:0x%08X\n", addr_to);
		
		memcpy((void*)addr_to, (void*)addr_from, PAGESIZE);
	}
}

/*
==================================================================================
 Funtion	:verify_vm_contents
 Input		:struct vm *dest
 		 < verify the contents *with >
 		 struct vm *with
 		 < verify with >
 Output		:void
 Return		:int
 		 < result >
 Description	:verify the vm contents
==================================================================================
*/
LOCAL int verify_vm_contents(struct vm *dest, struct vm *with)
{
	int i, j;
	
	for (i = 0;i < with->nr_pages;i++) {
		struct page *page_with = with->pages[i];
		struct page *page_dest = dest->pages[i];
		
		char *addr_with = (char*)page_to_address(page_with);
		char *addr_dest = (char*)page_to_address(page_dest);
		
		for (j = 0;j < PAGESIZE;j++) {
			if (*(addr_with + j) != *(addr_dest + j)) {
				return(1);
			}
		}
	}
	
	return(0);
}


/*
==================================================================================
 Funtion	:vm_file_map
 Input		:struct vm *vm
 		 < vm to map a file >
 		 unsigned long addr
 		 < file mapped virtual address >
 Output		:struct vm *vm
 		 < vm to map a file >
 Return		:int
 		 < result >
 Description	:map a page cache to vm
==================================================================================
*/
LOCAL int vm_file_map(struct vm *vm, unsigned long addr)
{
	struct vnode *vnode;
	struct page *page;
	struct page *page_cache;
	loff_t pos;
	loff_t index;
	unsigned long len = addr - vm->start;
	
	if (UNLIKELY(!vm->vnode)) {
		panic("0:unexpected error at %s[vm->vnode is NULL]\n", __func__);
		return(-EACCES);
	}
	
	vnode = vm->vnode;
	
	index = len / PAGESIZE;
	pos = (len + vm->offset) / PAGESIZE;
	
	page_cache = get_vm_page_cache(vnode, pos);
	
	page = page_cache;
	
#if 0
	/* -------------------------------------------------------------------- */
	/* last index ?								*/
	/* -------------------------------------------------------------------- */
	if (UNLIKELY(index == (vm->nr_pages - 1))) {
		unsigned long poff = vm->length & PAGE_UMASK;
		printf("new 0x%08X\n", vm->length);
		if (poff) {
			char *last_page;
			page = alloc_page();
			
			printf("new page assign!\n");
			
			if (UNLIKELY(!page)) {
				panic("0:unexpected error at %s\n", __func__);
				return(-ENOMEM);
			}
			
			copy_page_contents(page, page_cache);
			
			last_page = (char*)page_to_address(page);
			
			memset((void*)(last_page + poff), 0x00, PAGESIZE - poff);
		}
	}
#endif
	
	//printf("get_vm_page_cache 0x%08X 0x%08X 0x%08X 0x%08X\n", page, addr, pos, vm->offset);
	
	if (!page) {
		panic("1:unexpected error at %s\n", __func__);
		return(-ENOMEM);
	}

	if ((vm->mmap_flags & MAP_PRIVATE) && (vm->prot & PROT_WRITE)) {
		struct page *private;
		
		private = alloc_page();
		
		if (UNLIKELY(!private)) {
			panic("2:unexpected error at %s\n", __func__);
		}
		
		copy_page_contents(private, page);
		
		vm->pages[index] = private;
		
		free_page(page);
	} else {
		vm->pages[index] = page;
	}
	
	return(index);
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
