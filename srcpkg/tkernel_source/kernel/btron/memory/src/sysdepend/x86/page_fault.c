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
#include <bk/memory/vm.h>
#include <bk/memory/page_fault.h>
/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL void show_regs(struct ctx_reg *reg, uint32_t error_code,
			unsigned long fault_address);

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
 Funtion	:page_fault_handler
 Input		:uint32_t int_num
 		 < interrupt number >
 		 struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code >
 		 unsigned long fault_address
 		 < address at which fault occurs >
 Output		:void
 Return		:void
 Description	:handler for page fault
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPHANDLER void page_fault_handler(uint32_t int_num, struct ctx_reg *reg,
			uint32_t error_code, unsigned long fault_address)
{
static int count = 0;

	struct process *current = get_current();
	struct memory_space *mspace = current->mspace;
	static unsigned long before = 0;
	
	if (before == fault_address) {
		printf("same page fault address!!!\n");
		show_regs(reg, error_code, fault_address);
		show_pagetables(get_current(), fault_address, fault_address + 0x1000);
		show_vm_list(current);
		for(;;);
	}
	
	before = fault_address;
	
	if ((fault_address < PROCESS_SIZE) &&
		((error_code & ERROR_USER) || (error_code & ERROR_PRESENT) == 0)) {
//		error_code & ERROR_USER) {
		unsigned long stack_rlim = current->rlimits[RLIMIT_STACK].rlim_cur;
		struct vm *vm;
		int err;
		
		/* ------------------------------------------------------------ */
		/* extend stack							*/
		/* ------------------------------------------------------------ */
		if (mspace->end_stack &&
			((mspace->end_stack - stack_rlim) < fault_address)) {
			unsigned long extend_size;
			
			if (mspace->start_stack < fault_address) {
				//printf("COW stack\n");
				//printf("activate stack 0x%08X [%d]\n", fault_address, is_vm_cow(fault_address));
				/* -------------------------------------------- */
				/* cow by forked				*/
				/* -------------------------------------------- */
				if ((error_code & ERROR_WRITE) &&
						is_vm_cow(fault_address)) {
					
					goto activate_page;
				}
				printf("cow stack fault <pid>:%d\n", get_current()->pid);
				show_regs(reg, error_code, fault_address);
				show_pagetables(get_current(), fault_address, fault_address + 0x1000);
				//show_pagetables(current, mspace->start_stack, mspace->end_stack);
				//panic("1:unexpected error at %s [stack top:0x%08X, fault address:0x%08X]\n",
				//	__func__, mspace->start_stack, fault_address);
				for(;;);
			}
			extend_size = mspace->start_stack - fault_address;
			extend_size = RoundPage(extend_size);
			
			if (extend_size < PROC_STACK_SIZE) {
				extend_size = PROC_STACK_SIZE;
			}
			
			if (stack_rlim < ((mspace->end_stack - mspace->start_stack)
								 + extend_size)) {
				extend_size = stack_rlim -
					(mspace->end_stack - mspace->start_stack);
			}
			
			//printf("extend stack [0x%08X]\n", extend_size);
			//err = vm_extend_stack(current, PROC_STACK_SIZE);
			err = vm_extend_stack(current, extend_size);
			
			if (err) {
				show_regs(reg, error_code, fault_address);
				panic("2:unexpected error:vm_extend_stack at %s[0x%08X]\n", __func__, fault_address);
				goto failed_extend_stack;
			}
			
			/* stack expanded successfully				*/
			return;
		}
		
		if (mspace->start_stack < fault_address) {
			show_regs(reg, error_code, fault_address);
			panic("5:unexpected error %s[0x%08X]\n", __func__, fault_address);
		}
		
activate_page:
		/* ------------------------------------------------------------ */
		/* activate vm page						*/
		/* ------------------------------------------------------------ */
		vm = get_address_vm(current, fault_address, fault_address);
		
		if (UNLIKELY(!vm)) {
			show_regs(reg, error_code, fault_address);
			show_pagetables(current, fault_address & PAGE_MASK, (fault_address + 0x1000)& PAGE_MASK);
			//show_vm_list(current);
			printf("6:cannot find get_address_vm in page fault[0x%08X]\n", fault_address);
			//*(int*)0xFFFFFFFF = 0;
			for(;;);
			goto failed_activate_vm;
		}
		
		err = activate_vm_page(current, vm, fault_address, error_code);
		
		if (UNLIKELY(err)) {
			show_regs(reg, error_code, fault_address);
			show_vm_list(current);
			panic("7:page fault panic\n");
			goto failed_activate_vm;
		}
		
		return;
	}

failed_activate_vm:
failed_extend_stack:
	if (1 <= count) {
		for(;;);
	} else {
		printf("8:unexpected page fault\n");
		show_regs(reg, error_code, fault_address);
		count++;
	}
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
 Funtion	:show_regs
 Input		:struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code >
 		 unsigned long fault_address
 		 < address at which fault occurs >
 Output		:void
 Return		:void
 Description	:show fault information
==================================================================================
*/
LOCAL void show_regs(struct ctx_reg *reg, uint32_t error_code,
			unsigned long fault_address)
{
	printf("page fault[0x%08X]\n", fault_address);
#if 1
	printf("error code:%u\n", error_code);
	printf("eax:0x%08X ", reg->eax);
	printf("ebx:0x%08X ", reg->ebx);
	printf("ecx:0x%08X ", reg->ecx);
	printf("edx:0x%08X\n", reg->edx);
	printf("eip:0x%08X ", reg->eip);
	printf("eflags:0x%08X ", reg->eflags);
	printf("cs:0x%08X ", reg->cs);
	printf("esp:0x%08X\n", reg->esp);
	printf("ss:0x%08X ", reg->ss);
	
	printf("*eip:0x%08X\n", *(unsigned long*)(reg->eip));
#endif
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
