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


#include <cpu.h>

#include <bk/kernel.h>
#include <bk/tls.h>

#include <t2ex/string.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int get_free_tls(struct task_context_block *tskctxb);
LOCAL INLINE int test_tls_empy(struct segment_desc *tls_desc);
LOCAL INLINE int test_tls_zero(struct segment_desc *tls_desc);

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
 Funtion	:set_thread_area
 Input		:struct user_desc *u_info
 		 < thread area information to set >
 Output		:void
 Return		:int
 		 < result >
 Description	:set a GDT entry for thread-local storage
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int set_thread_area(struct user_desc *u_info)
{
	struct task *task;
	struct user_desc udesc;
	int index;
#if 0
	printf("set_thread_area:\n");
	printf("[entry_number=%d, ", u_info->entry_number);
	printf("base_adddr=0x%08X, ", u_info->base_addr);
	printf("limit=0x%08X, ", u_info->limit);
	printf("seg_32bit=%d, ", u_info->seg_32bit);
	printf("contents=0x%X, ", u_info->contents);
	printf("read_exec_only=%d, ", u_info->read_exec_only);
	printf("limit_in_pages=%d, ", u_info->limit_in_pages);
	printf("seg_not_present=%d, ", u_info->seg_not_present);
	printf("useable=%d]\n", u_info->useable);
#endif
	if (UNLIKELY(!u_info)) {
		return(-EFAULT);
	}
	
	if(UNLIKELY(!copy_form_user(&udesc, u_info, sizeof(struct user_desc)))) {
		return(-EFAULT);
	}
	
	if (udesc.entry_number == -1) {
		int err;
		/* ------------------------------------------------------------ */
		/* try to allocate free tls					*/
		/* ------------------------------------------------------------ */
		index = get_free_tls(&get_current_task()->tskctxb);
		index += TLS_BASE_ENTRY;
		err = ChkSpaceRW(u_info, sizeof(struct user_desc));
		if (err) {
			return(-EFAULT);
		}
		u_info->entry_number = index;
		udesc.entry_number = index;
	} else {
		index = udesc.entry_number;
	}
	
	if (UNLIKELY(MAX_NUM_DESCRIPTOR < index ||
				index < TLS_BASE_ENTRY)) {
		return(-EINVAL);
	}

	if (!udesc.seg_32bit) {
		return(-EINVAL);
	}
	
	if (1 < udesc.contents) {
		return(-EINVAL);
	}
	
	if (udesc.seg_not_present) {
		return(-EINVAL);
	}
	
	task = get_current_task();
	
	/* -------------------------------------------------------------------- */
	/* set user descriptor to task's tsl					*/
	/* -------------------------------------------------------------------- */
	set_user_desc_to_seg_desc(task, &udesc);
	/* -------------------------------------------------------------------- */
	/* update ldt entry in gdt						*/
	/* -------------------------------------------------------------------- */
	update_tls_descriptor(task, udesc.entry_number);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_thread_area
 Input		:struct user_desc *u_info
 		 < thread area information to get >
 Output		:struct user_desc *u_info
 		 < thread area information to get >
 Return		:int
 		 < result >
 Description	:get a GDT entry for thread-local storage
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int get_thread_area(struct user_desc *u_info)
{
	int err;
	struct user_desc udesc;
	struct task *task;
	int gdt_index;
	int index;
	
	printf("get_thread_area:\n");
	
	if (UNLIKELY(!u_info)) {
		return(-EFAULT);
	}
	
	err = ChkUsrSpaceR(u_info, sizeof(struct user_desc));
	
	if (err) {
		return(-EFAULT);
	}
	
	gdt_index = u_info->entry_number;
	
	if (UNLIKELY(MAX_NUM_DESCRIPTOR < gdt_index ||
				gdt_index < TLS_BASE_ENTRY)) {
		return(-EINVAL);
	}
	
	index = u_info->entry_number - TLS_BASE_ENTRY;
	
	task = get_current_task();
	
	if (UNLIKELY(test_tls_zero(&task->tskctxb.tls_desc[index]))) {
		u_info->read_exec_only = 1;
		u_info->seg_not_present = 1;
		return(0);
	}
	
	if (UNLIKELY(test_tls_empy(&task->tskctxb.tls_desc[index]))) {
		u_info->read_exec_only = 1;
		u_info->seg_not_present = 1;
		return(0);
	}
	
	get_user_desc_from_seg_desc(task, &udesc);
	
#if 1
	printf("[entry_number=%d, ", u_info->entry_number);
	printf("base_adddr=0x%08X, ", u_info->base_addr);
	printf("limit=0x%08X, ", u_info->limit);
	printf("seg_32bit=%d, ", u_info->seg_32bit);
	printf("contents=0x%X, ", u_info->contents);
	printf("read_exec_only=%d, ", u_info->read_exec_only);
	printf("limit_in_pages=%d, ", u_info->limit_in_pages);
	printf("seg_not_present=%d, ", u_info->seg_not_present);
	printf("useable=%d]\n", u_info->useable);
#endif
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
 Funtion	:test_tls_empty
 Input		:struct segment_desc *tls_desc
 		 < tls segment descriptor to test >
 Output		:void
 Return		:int
 		 < bool result >
 Description	:test whether tls segment descriptor is empty or not
==================================================================================
*/
LOCAL INLINE int test_tls_empy(struct segment_desc *tls_desc)
{
	unsigned long test = (TLS_PRESENT | TLS_NO_READ_EXEC_ONLY);
	
	if (tls_desc->hi & test) {
		if (tls_desc->hi & ~test) {
			/* not empty						*/
			return(0);
		}
		
		return(1);
	}
	
	if (tls_desc->hi & ~test) {
		return(0);
	}
	
	return(1);
}

/*
==================================================================================
 Funtion	:test_tls_zero
 Input		:struct segment_desc *tls_desc
 		 < tls segment descriptor to test >
 Output		:void
 Return		:int
 		 < bool result >
 Description	:test whether tls segment descriptor is zeroed or not
==================================================================================
*/
LOCAL INLINE int test_tls_zero(struct segment_desc *tls_desc)
{
	return(!tls_desc->hi && !tls_desc->low);
}

/*
==================================================================================
 Funtion	:get_free_tls
 Input		:struct task_context_block *tskctxb
 		 < task context block to search free TLS >
 Output		:void
 Return		:int
 		 < index number of free TLS >
 Description	:search free TLS
==================================================================================
*/
LOCAL int get_free_tls(struct task_context_block *tskctxb)
{
	int i;
	struct segment_desc *tls = &tskctxb->tls_desc[0];
	
	for (i = 0;i < NR_TLS_ENTRYIES;i++) {
		if (test_tls_zero(&tls[i]) || test_tls_empy(&tls[i])) {
			tls->hi = 0;
			tls->low = 0;
			return(i);
		}
	}
	
	return(-ESRCH);
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
