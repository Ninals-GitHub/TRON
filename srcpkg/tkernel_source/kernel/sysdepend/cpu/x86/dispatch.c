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
#include <bk/kernel.h>

#include <debug/vdebug.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
IMPORT void low_pow( void );	// symgr.h
void next_to(void);
LOCAL INLINE unsigned long stack_up(unsigned long stack, unsigned long value);


/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	ON_EXE_EAX		0
#define	ON_EXE_EBX		0
#define	ON_EXE_ECX		0
#define	ON_EXE_EDX		0
#define	ON_EXE_EDI		0
#define	ON_EXE_ESI		0

#define	INIT_TASK_EBX		0
#define	INIT_TASK_ECX		0
#define	INIT_TASK_EDX		0
#define	INIT_TASK_ESI		0
#define	INIT_TASK_EDI		0
#define	INIT_TASK_EBP		0
#define	INIT_TASK_EAX		0	// unused
#define	INIT_TASK_DS		0	// unused
#define	INIT_TASK_ES		0	// unused
#define	INIT_TASK_FS		0
#define	INIT_TASK_GS		0
#define	INIT_TASK_ORIG_EAX	0
#define	INIT_TASK_IP		0	// unused
#define	INIT_TASK_CS		0	// unused
#define	INIT_TASK_FLAGS		0	// unused
#define	INIT_TASK_SP		0	// unused
#define	INIT_TASK_SS		0	// unused

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
 Funtion	:start_task
 Input		:unsigned long start_text
 		 < start address >
 Output		:void
 Return		:int
 		 < result >
 Description	:execute new user process forcibly
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int start_task(unsigned long start_text)
{
	struct process *current = get_current();
	struct task *current_task = get_current_task();
	struct task_context_block *current_ctxb = get_current_ctxb();

	
	if (KERNEL_BASE_ADDR <= start_text) {
		return(-EINVAL);
	}
	
	current_task->tskatr |= TA_RNG3;
	
	/* -------------------------------------------------------------------- */
	/* iret flag is cleared							*/
	/* -------------------------------------------------------------------- */
	current_ctxb->need_iret = FALSE;
	
	/* -------------------------------------------------------------------- */
	/* setup user task context						*/
	/* -------------------------------------------------------------------- */
	current_ctxb->ip = start_text;
	current_ctxb->ds = SEG_USER_DS;
	current_ctxb->es = SEG_USER_DS;
	current_ctxb->gs = SEG_USER_DS;
	current_ctxb->fs = SEG_USER_DS;
	current_ctxb->sysenter_cs = SEG_USER_CS;

	current_ctxb->sp = current->mspace->end_stack;
	
	dispatch_disabled = DDS_ENABLE;
	
	update_tss_esp0(getEsp());
	
	/* -------------------------------------------------------------------- */
	/* execute in user space						*/
	/* -------------------------------------------------------------------- */
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
		"movw %[ds], %%ds		\n\t"
		"movl %[init_eax], %%eax	\n\t"
		"movl %[init_ebx], %%ebx	\n\t"
		"movl %[init_ecx], %%ecx	\n\t"
		"movl %[init_edx], %%edx	\n\t"	// rtdl_fini
		"movl %[init_edi], %%edi	\n\t"
		"movl %[init_esi], %%esi	\n\t"
		"cld				\n\t"
		"iret				\n\t"
		:
		:[fs]"m"(current_ctxb->fs), [gs]"m"(current_ctxb->gs),
		 [es]"m"(current_ctxb->es), [ds]"m"(current_ctxb->ds),
		 [cs]"m"(current_ctxb->sysenter_cs), [esp]"m"(current_ctxb->sp),
		 [start_func]"m"(current_ctxb->ip),
		 [eflags]"i"(EFLAGS_ID | EFLAGS_IF | EFLAGS_IOPL),
		 [init_eax]"i"(ON_EXE_EAX), [init_ebx]"i"(ON_EXE_EBX),
		 [init_ecx]"i"(ON_EXE_ECX), [init_edx]"i"(ON_EXE_EDX),
		 [init_edi]"i"(ON_EXE_EDI), [init_esi]"i"(ON_EXE_ESI)
		 :"memory"
	);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:force_dispatch
 Input		:void
 Output		:void
 Return		:void
 Description	:Throw away the current task context. and forcibly dispatch to
 		 the task that should be performed next.
 		 Use at kernel task dispatch, system startup, 'tk_ext_tsk and
 		 tk_exd_tsk.'
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void force_dispatch(void)
{
	W rng;
	CTXB *next_ctxb = &schedtsk->tskctxb;
	CTXB *current_ctxb = NULL;
	struct pt_regs *regs;
	unsigned long *img;
	
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
	regs = next_ctxb->pt_regs;
	
	/* -------------------------------------------------------------------- */
	/* forcibly dispatch							*/
	/* -------------------------------------------------------------------- */
	if (rng) {
		if (next_ctxb->forked) {
			next_ctxb->forked = FALSE;
			//printf("forked[pid=%d, ", schedtsk->proc->pid);
			if (schedtsk->set_child_tid) {
				copy_to_user((void*)schedtsk->set_child_tid,
						(const void*)&schedtsk->tskid,
						sizeof(int));
			}
		}
		update_tss_esp0(getEsp());
		ASM(
			"pushl	%[reg_ss]		\n\t"
			"pushl	%[reg_sp]		\n\t"
			"pushl	%[reg_fl]		\n\t"
			"pushl	%[reg_cs]		\n\t"
			"pushl	%[reg_ip]		\n\t"
			"movl	%[regs_ptr], %%eax	\n\t"
			"movl	(%%eax), %%ebx		\n\t"	// bx
			"movl	4(%%eax), %%ecx		\n\t"	// cx
			"movl	8(%%eax), %%edx		\n\t"	// dx
			"movl	12(%%eax), %%esi	\n\t"	// si
			"movl	16(%%eax), %%edi	\n\t"	// di
			"movl	20(%%eax), %%ebp	\n\t"	// bp
			"movw	32(%%eax), %%es		\n\t"	// es
			"movw	36(%%eax), %%fs		\n\t"	// fs
			"movw	40(%%eax), %%gs		\n\t"	// gs
			"movw	28(%%eax), %%ds		\n\t"	// ds
			"movl	24(%%eax), %%eax	\n\t"	// ax
			"cld				\n\t"
			"iret				\n\t"
			:
			:[reg_ss]"m"(regs->ss), [reg_sp]"m"(regs->sp),
			 [reg_fl]"m"(regs->flags),
			 [reg_cs]"m"(regs->cs), [reg_ip]"m"(regs->ip),
			 [regs_ptr]"a"(regs)
			:"memory"
		);
	} else {
		ASM (
			"movl	%[reg_sp], %%esp	\n\t"
			"pushl	%[reg_fl]		\n\t"
			"pushl	%[reg_cs]		\n\t"
			"pushl	%[reg_ip]		\n\t"
			"movl	%[regs_ptr], %%eax	\n\t"
			"movl	(%%eax), %%ebx		\n\t"	// bx
			"movl	4(%%eax), %%ecx		\n\t"	// cx
			"movl	8(%%eax), %%edx		\n\t"	// dx
			"movl	12(%%eax), %%esi	\n\t"	// si
			"movl	16(%%eax), %%edi	\n\t"	// di
			"movl	20(%%eax), %%ebp	\n\t"	// bp
			"movw	32(%%eax), %%es		\n\t"	// es
			"movw	36(%%eax), %%fs		\n\t"	// fs
			"movw	40(%%eax), %%gs		\n\t"	// gs
			"movw	28(%%eax), %%ds		\n\t"	// ds
			"movl	24(%%eax), %%eax	\n\t"	// ax
			"cld				\n\t"
			"iretl				\n\t"
			:
			:[reg_sp]"m"(next_ctxb->ssp),
			 [reg_fl]"m"(regs->flags),
			 [reg_cs]"m"(regs->cs), [reg_ip]"m"(regs->ip),
			 [regs_ptr]"a"(regs)
			:"memory"
		);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_context
 Input		:TCB *tcb
 		 < task control block to be set up >
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
		//ctxb->sp = (unsigned long)tcb->istack;
	}

	ctxb->cr2 = 0;
	ctxb->trap_nr = 0;
	ctxb->error_code = 0;
	ctxb->io_bitmap = NULL;
	ctxb->iopl = 0;
	ctxb->io_bitmap_max = 0;
	
	ctxb->need_iret = TRUE;
	ctxb->forked = FALSE;
	/* -------------------------------------------------------------------- */
	/* eflags								*/
	/* -------------------------------------------------------------------- */
	if (!rng) {
		ctxb->flags = EFLAGS_ID | EFLAGS_IF | EFLAGS_IOPL;
	} else {
		ctxb->flags = EFLAGS_ID | EFLAGS_IF;
	}
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
	struct process *current_proc;
	struct process *next_proc;
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
		current_proc = get_current();
		next_proc = get_scheduled();
		
		if (current_proc->mspace != next_proc->mspace) {
			//printf("switched ms. pid=%d -> pid=%d\n", current_proc->pid, next_proc->pid);
			switch_ms(current_proc, next_proc);
		}
		
		if (UNLIKELY(next_ctxb->need_iret)) {
			unsigned long *esp0 = (unsigned long*)get_tss_esp0();
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
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_stacd
 Input		:struct task *tcb
 		 < tcb to be set up >
 		 INT stacd
 		 < stacd to set up >
 Output		:void
 Return		:void
 Description	:set up stacd for T-Kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void setup_stacd( struct task *tcb, INT stacd )
{
	//W rng;
	unsigned long *sp;
	struct pt_regs initial_regs = {
		.bx = INIT_TASK_EBX,
		.cx = INIT_TASK_ECX,
		.dx = INIT_TASK_EDX,
		.si = INIT_TASK_ESI,
		.di = INIT_TASK_EDI,
		.bp = INIT_TASK_EBP,
		.ax = INIT_TASK_EAX,
		.ds = INIT_TASK_DS,
		.es = INIT_TASK_ES,
		.fs = INIT_TASK_FS,
		.gs = INIT_TASK_GS,
		.orig_ax = INIT_TASK_ORIG_EAX,
		.ip = INIT_TASK_IP,
		.cs = INIT_TASK_CS,
		.flags = INIT_TASK_FLAGS,
		.sp = INIT_TASK_SP,
		.ss = INIT_TASK_SS
	};
	
	//rng = (tcb->tskatr & TA_RNG3) >> 8;
	
	//if (rng) {
	//	sp = (unsigned long*)tcb->tskctxb.sp;
	//} else {
		sp = (unsigned long*)tcb->tskctxb.ssp;
	//}
	
	/* -------------------------------------------------------------------- */
	/* setup initial context on iret					*/
	/* this contex is overriden by fork()					*/
	/* -------------------------------------------------------------------- */
	*(sp--) = tcb->tskctxb.ds;			/* pt_regs.ss		*/
	*(sp--) = tcb->tskctxb.sp;			/* pt_regs.sp		*/
	*(sp--) = tcb->tskctxb.flags;			/* pt_regs.flags	*/
	*(sp--) = tcb->tskctxb.sysenter_cs;		/* pt_regs.cs		*/
	*(sp--) = tcb->tskctxb.ip;			/* pt_regs.ip		*/
	*(sp--) = initial_regs.orig_ax;			/* pt_regs.orig_ax	*/
	*(sp--) = initial_regs.gs;			/* pt_regs.gs		*/
	*(sp--) = initial_regs.fs;			/* pt_regs.fs		*/
	*(sp--) = tcb->tskctxb.ds;			/* pt_regs.es		*/
	*(sp--) = tcb->tskctxb.ds;			/* pt_regs.ds		*/
	*(sp--) = initial_regs.ax;			/* pt_regs.ax		*/
	*(sp--) = initial_regs.bp;			/* pt_regs.bp		*/
	*(sp--) = initial_regs.di;			/* pt_regs.di		*/
	*(sp--) = initial_regs.si;			/* pt_regs.si		*/
	*(sp--) = initial_regs.dx;			/* pt_regs.dx		*/
	*(sp--) = initial_regs.cx;			/* pt_regs.cx		*/
	*(sp--) = initial_regs.bx;			/* pt_regs.bx		*/
	
	tcb->tskctxb.pt_regs = (struct pt_regs*)(sp + 1);
	
	tcb->tskctxb.ssp = (void*)((unsigned long)tcb->tskctxb.ssp
				- sizeof(struct pt_regs));

	/* -------------------------------------------------------------------- */
	/* setup arguments for T-Kernel						*/
	/* -------------------------------------------------------------------- */
	*sp = (VW)tcb->exinf;
	sp--;
	*sp = stacd;
	sp--;
	
	//if (rng) {
	//	tcb->tskctxb.sp  -= sizeof(unsigned long) * 2;
	//} else {
		tcb->tskctxb.ssp -= sizeof(unsigned long) * 2;
	//}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_task_context
 Input		:struct task *from
 		 < copy from >
 		 struct task *new
 		 < copy to >
 Output		:void
 Return		:int
 		 < resutl >
 Description	:copy current task context to the new one
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_task_context(struct task *from, struct task *new)
{
	int rng = (new->tskatr & TA_RNG3) >> 8;
	struct task_context_block *new_ctx = &new->tskctxb;
	struct task_context_block *from_ctx = &from->tskctxb;
	int i;
	
	if (!rng) {
		return(-ENOSYS);
	}
	
	new_ctx->sp0 = (unsigned long)new->isstack;
	new_ctx->ssp = (void*)new->isstack;
	
	new_ctx->ds = from_ctx->ds;
	new_ctx->es = from_ctx->es;
	new_ctx->fs = from_ctx->fs;
	new_ctx->sysenter_cs = from_ctx->sysenter_cs;

	//new_ctx->sp = from_ctx->sp;
	
	new_ctx->cr2 = 0;
	new_ctx->trap_nr = 0;
	new_ctx->error_code = 0;
	new_ctx->io_bitmap = NULL;
	new_ctx->iopl = 0;
	new_ctx->io_bitmap_max = 0;
	
	new_ctx->need_iret = TRUE;
	
	new_ctx->ssp = (void*)new_ctx->sp0;
	
	memcpy((void*)new_ctx->tls_desc, (void*)from_ctx->tls_desc,
			sizeof(struct segment_desc) * NR_TLS_ENTRYIES);
#if 0
	for (i = 0;i < NR_TLS_ENTRYIES;i++) {
		printf("tls[%d]:0x%08X 0x%08X", i, new_ctx->tls_desc[i].hi, new_ctx->tls_desc[i].low);
	}
#endif
	new_ctx->forked = TRUE;
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_fork_user_context
 Input		:struct task *new
 		 < forked task >
 		 struct pt_regs *regs
 		 < context for new task >
 Output		:void
 Return		:void
 Description	:setup forked task context
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void setup_fork_user_context(struct task *new, struct pt_regs *regs)
{
	struct pt_regs *stacked_regs;
	
	stacked_regs = new->tskctxb.pt_regs;
	
	/* -------------------------------------------------------------------- */
	/* setup stack								*/
	/* -------------------------------------------------------------------- */
	memcpy((void*)stacked_regs, (void*)regs, sizeof(struct pt_regs));
	
	/* -------------------------------------------------------------------- */
	/* setup the result of child fork()					*/
	/* -------------------------------------------------------------------- */
	stacked_regs->ax = 0;
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
 Funtion	:stack_up
 Input		:unsigned long stack
 		 < address of stack >
 		 unsigned long value
 		 < value to stack up >
 Output		:void
 Return		:unsigned long
 		 < updated stack top address >
 Description	:stack up the value
==================================================================================
*/
LOCAL INLINE unsigned long stack_up(unsigned long stack, unsigned long value)
{
	stack -= sizeof(unsigned long);
	
	*(unsigned long*)stack = value;
	
	return(stack);
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
