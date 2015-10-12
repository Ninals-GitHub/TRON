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

#include <tk/typedef.h>
#include <tk/kernel.h>
#include <tk/task.h>
#include <cpu.h>

#include <debug/vdebug.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
IMPORT void low_pow( void );	// symgr.h
//LOCAL void next_to(void);
void next_to(void);


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
IMPORT TCB	*ctxtsk;
IMPORT TCB	*schedtsk;

IMPORT INT	dispatch_disabled;
IMPORT UINT	lowpow_discnt;


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:Throw away the current task context. and forcibly dispatch to
 		 the task that should be performed next.
 		 Use at system startup and 'tk_ext_tsk, tk_exd_tsk.'
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void force_dispatch(void)
{
	W rng;
	CTXB *next_ctxb = &schedtsk->tskctxb;
	CTXB *current_ctxb = NULL;
	
	rng = (schedtsk->tskatr & TA_RNG3) >> 8;
	
	if (ctxtsk) {
		current_ctxb = &ctxtsk->tskctxb;
	}
	
	/* -------------------------------------------------------------------- */
	/* iret flag is cleared							*/
	/* -------------------------------------------------------------------- */
	next_ctxb->need_iret = FALSE;
	
	/* -------------------------------------------------------------------- */
	/* switch fs and gs context						*/
	/* -------------------------------------------------------------------- */
	if (ctxtsk) {
		ASM (
			"movw %%fs, %[current_fs]		\n\t"
			"movw %%gs, %[current_gs]		\n\t"
			:[current_fs]"=m"(current_ctxb->fs),
			 [current_gs]"=m"(current_ctxb->gs)
			:
			:"memory"
		);
	}

	/* -------------------------------------------------------------------- */
	/* update current task							*/
	/* -------------------------------------------------------------------- */
	ctxtsk = schedtsk;
	dispatch_disabled = DDS_ENABLE;
	
	/* -------------------------------------------------------------------- */
	/* forcibly dispatch							*/
	/* -------------------------------------------------------------------- */
	if (rng) {
		update_tss_esp0(getEsp());

		ASM (
			"movw %[fs], %%fs		\n\t"
			"movw %[gs], %%gs		\n\t"
			"movw %[es], %%es		\n\t"
			"movl %[esp], %%eax		\n\t"
			"pushl %[ds]			\n\t"
			"pushl %%eax			\n\t"
			"pushl %[eflags]		\n\t"
			"pushl %[cs]			\n\t"
			"pushl %[start_func]		\n\t"
			"cld				\n\t"
			"iret				\n\t"
			:
			:[fs]"m"(next_ctxb->fs), [gs]"m"(next_ctxb->gs),
			 [es]"m"(next_ctxb->es), [ds]"m"(next_ctxb->ds),
			 [cs]"m"(next_ctxb->sysenter_cs), [esp]"m"(next_ctxb->sp),
			 [start_func]"m"(next_ctxb->ip),
			 [eflags]"i"(EFLAGS_IF)
			:"eax","memory"
		);
	} else {
		ASM (
			"movl %[esp], %%esp		\n\t"
			"pushl %[eflags]		\n\t"
			"pushl %[cs]			\n\t"
			"pushl %[start_func]		\n\t"
			"cld				\n\t"
			"iret				\n\t"
			:
			:[cs]"m"(next_ctxb->sysenter_cs), [esp]"m"(next_ctxb->ssp),
			 [start_func]"m"(next_ctxb->ip),
			 [eflags]"i"(EFLAGS_IF)
			:"eax","memory"
		);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_context
 Input		:void
 Output		:void
 Return		:void
 Description	:Create stack frame for task startup Call from 'make_dormant()'
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void setup_context( TCB *tcb )
{
	W	rng = (tcb->tskatr & TA_RNG3) >> 8;
	CTXB	*ctxb = &tcb->tskctxb;

	ctxb->sp0 = (unsigned long)tcb->isstack;
	 
	ctxb->ip = (unsigned long)tcb->task;
	ctxb->ssp = (void*)tcb->isstack;

	if (!rng) {
		ctxb->ds = SEG_KERNEL_DS;
		ctxb->es = SEG_KERNEL_DS;
		ctxb->gs = SEG_KERNEL_DS;
		ctxb->fs = SEG_KERNEL_DS;
		ctxb->sysenter_cs = SEG_KERNEL_CS;
		ctxb->sp = (unsigned long)tcb->isstack;
	} else {
		ctxb->ds = SEG_USER_DS;
		ctxb->es = SEG_USER_DS;
		ctxb->gs = SEG_USER_DS;
		ctxb->fs = SEG_USER_DS;
		ctxb->sysenter_cs = SEG_USER_CS;
		ctxb->sp = (unsigned long)tcb->istack;
	}

	ctxb->cr2 = 0;
	ctxb->trap_nr = 0;
	ctxb->error_code = 0;
	ctxb->io_bitmap = NULL;
	ctxb->iopl = 0;
	ctxb->io_bitmap_max = 0;
	
	ctxb->need_iret = TRUE;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dispatch
 Input		:void
 Output		:void
 Return		:void
 Description	:context switch
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void dispatch(void)
{
	CTXB	*current_ctxb;
	CTXB	*next_ctxb;
	
	//vd_printf("dispatch current %s ->", ctxtsk->name);
#if 0
	vd_printf("%s ->", ctxtsk->name);
	if (schedtsk) {
		vd_printf("%s ", schedtsk->name);
	} else {
		vd_printf("NULL\n");
	}
#endif
	
wake_up_from_low_power:
	if (schedtsk == ctxtsk) {
		return;
	}
	
	DISABLE_INTERRUPT {
		dispatch_disabled = DDS_DISABLE;
		
		if (UNLIKELY(!schedtsk)) {
			if (!lowpow_discnt) {
				//vd_printf("enter low power mode[%s]\n", ctxtsk->name);
				low_pow();
			}
			
			dispatch_disabled = DDS_ENABLE;
			ENABLE_INTERRUPT;
			hlt();
			goto wake_up_from_low_power;
		}
		
		current_ctxb = &ctxtsk->tskctxb;
		next_ctxb = &schedtsk->tskctxb;
		BARRIER();
		if (next_ctxb->need_iret) {
			unsigned long *esp0 = get_tss_esp0();
			//vd_printf("iret next : %s cur_esp:0x%08X nxt_esp:0x%08X\n", schedtsk->name, current_ctxb->ssp, next_ctxb->ssp);
			ASM (
			"pushfl					\n\t"
			"pushl	%%ebp				\n\t"
			"movl	%%esp, %[current_sp]		\n\t"
			"movl	$2f, %[current_ip]		\n\t"
			"jmp force_dispatch			\n\t"
		"2:						\t"
			"movl	%%esp, %[ssp0]			\n"
			"popl	%%ebp				\n\t"
			"popfl					\n\t"
			:[current_sp]"=m"(current_ctxb->ssp),
			 [current_ip]"=m"(current_ctxb->ip),
			 [ssp0]"=m"(esp0)
			);
			/* ---------------------------------------------------- */
			/* update tss						*/
			/* ---------------------------------------------------- */
			update_tss_esp0(getEsp());
					if (!schedtsk)vd_printf("next2 schedtsk is null!!\n" );
			ctxtsk = schedtsk;
			dispatch_disabled = DDS_ENABLE;
			//ENABLE_INTERRUPT;
			//vd_printf("enter next2 %s cur_esp:0x%08X\n", ctxtsk->name, ctxtsk->tskctxb.ssp);
			return;
		}
#if 0
		if (schedtsk->name[0] == '0') {
			vd_printf("before disp c:0x%08X to ", ctxtsk );
			vd_printf("s:0x%08X ", schedtsk);
			vd_printf("sp:0x%08X\n", current_ctxb->ssp);
			vd_printf("next ip:0x%08X", next_ctxb->ip);
			vd_printf(" sp;0x%08X\n", next_ctxb->ssp);
		}
#endif
		BARRIER();
		/* ------------------------------------------------------------ */
		/* as for now switch memory context is not implemented		*/
		/* ------------------------------------------------------------ */
		
		/* ------------------------------------------------------------ */
		/* switch sp context and prepare to switch ip context		*/
		/* ------------------------------------------------------------ */
		ASM (
		"pushfl					\n\t"
		"pushl	%%ebp				\n\t"
		"movl	%%esp, %[current_sp]		\n\t"
		"movl	%[next_sp], %%esp		\n\t"
		"movl	$1f, %[current_ip]		\n\t"
		"pushl	%[next_ip]			\n\t"
		"jmp next_to				\n"
"1:							\t"
		"popl	%%ebp				\n\t"
		"popfl					\n\t"
		:[current_sp]"=m"(ctxtsk->tskctxb.ssp),
		 [current_ip]"=m"(ctxtsk->tskctxb.ip)
		:[next_sp]"m"(schedtsk->tskctxb.ssp), [next_ip]"m"(schedtsk->tskctxb.ip)
		:"esp", "memory"
		);
		
		dispatch_disabled = DDS_ENABLE;
		//if (schedtsk->name[0] == '0') {
			//vd_printf("before disp c:0x%08X to ", ctxtsk );
			//vd_printf("s:0x%08X ", schedtsk);
			//vd_printf("sp:0x%08X\n", schedtsk->tskctxb.ssp);
			//vd_printf("next ip:0x%08X", next_ctxb->ip);
			//vd_printf(" sp;0x%08X\n", next_ctxb->ssp);
		//}
		ctxtsk = schedtsk;
		//if (ctxtsk->name[0] == '0')
		//		vd_printf("enter next1 %s cur_esp:0x%08X\n", ctxtsk->name, getEsp());

		

	} //ENABLE_INTERRUPT;
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

/*
==================================================================================
 Funtion	:next_to
 Input		:void
 Output		:void
 Return		:void
 Description	:switch to next context completely
==================================================================================
*/
EXPORT void next_to(void)
{
	/* -------------------------------------------------------------------- */
	/* as for now switch fpu is not implemented 				*/
	/* -------------------------------------------------------------------- */
	
	/* -------------------------------------------------------------------- */
	/* update esp0 of tss by this esp					*/
	/* -------------------------------------------------------------------- */
	update_tss_esp0(getEsp());
	
	/* -------------------------------------------------------------------- */
	/* switch fs and gs context						*/
	/* -------------------------------------------------------------------- */
	ASM (
		"movw %%fs, %[current_fs]		\n\t"
		"movw %%gs, %[current_gs]		\n\t"
		"movw %[next_fs], %%fs			\n\t"
		"movw %[next_gs], %%gs			\n\t"
		:[current_fs]"=m"(ctxtsk->tskctxb.fs),
		 [current_gs]"=m"(ctxtsk->tskctxb.gs)
		:[next_fs]"m"(schedtsk->tskctxb.fs), [next_gs]"m"(schedtsk->tskctxb.gs)
		:"memory"
	);
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
