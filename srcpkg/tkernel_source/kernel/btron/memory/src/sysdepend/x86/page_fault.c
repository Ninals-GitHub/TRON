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
#define	ERROR_PRESENT		0x00000001
#define	ERROR_WRITE		0x00000002
#define	ERROR_USER		0x00000004
#define	ERROR_RSV_WRITE		0x00000008	/* caused by reading a 1 in	*/
						/* a reserved field		*/
#define	ERROR_I_FETCH		0x00000010	/* caused by instruction fetch	*/

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
	int err;
	
	if ((fault_address < PROCESS_SIZE) && (error_code & ERROR_USER)) {
		unsigned long stack_rlim = current->rlimits[RLIMIT_STACK].rlim_cur;
		
		/* ------------------------------------------------------------ */
		/* extend stack							*/
		/* ------------------------------------------------------------ */
		if ((mspace->end_stack - stack_rlim) < fault_address) {
		
			err = vm_extend_stack(current, PROC_STACK_SIZE);
			
			if (err) {
				goto failed_extend_stack;
			}
			
			/* stack expanded successfully				*/
			return;
		}
	}

failed_extend_stack:
	if (1 <= count) {
		
	} else {
		vd_printf("page fault[0x%08X]\n", fault_address);
		vd_printf("error code:%u\n", error_code);
		vd_printf("eax:0x%08X ", reg->eax);
		vd_printf("ebx:0x%08X ", reg->ebx);
		vd_printf("ecx:0x%08X\n", reg->ecx);
		vd_printf("eip:0x%08X ", reg->eip);
		vd_printf("eflags:0x%08X ", reg->eflags);
		vd_printf("cs:0x%08X\n", reg->cs);
		vd_printf("esp:0x%08X ", reg->esp);
		vd_printf("ss:0x%08X\n", reg->ss);

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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
