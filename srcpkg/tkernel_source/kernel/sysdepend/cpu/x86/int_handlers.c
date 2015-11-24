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

#include <stdint.h>
#include <tk/typedef.h>
#include <tk/errno.h>
#include <cpu.h>

#include <debug/vdebug.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	Default exception handlers without error code
----------------------------------------------------------------------------------
*/
LOCAL void handle_division_by_0(struct ctx_reg *reg);
LOCAL void handle_debug(struct ctx_reg *reg);
LOCAL void handle_nmi(struct ctx_reg *reg);
LOCAL void handle_breakpoint(struct ctx_reg *reg);
LOCAL void handle_int0(struct ctx_reg *reg);
LOCAL void handle_bound(struct ctx_reg *reg);
LOCAL void handle_invalid_opcode(struct ctx_reg *reg);
LOCAL void handle_cp_not_available(struct ctx_reg *reg);
LOCAL void handle_cp_seg_overrun(struct ctx_reg *reg);
LOCAL void handle_math_fault(struct ctx_reg *reg);
LOCAL void handle_alignment_check(struct ctx_reg *reg);
LOCAL void handle_machine_check(struct ctx_reg *reg);
LOCAL void handle_simd_fp(struct ctx_reg *reg);
/*
----------------------------------------------------------------------------------
	Default exception handlers with error code
----------------------------------------------------------------------------------
*/
LOCAL void handle_double_fault(struct ctx_reg *reg, uint32_t error_code);
LOCAL void handle_tss_error(struct ctx_reg *reg, uint32_t error_code);
LOCAL void handle_segment_not_present(struct ctx_reg *reg, uint32_t error_code);
LOCAL void handle_stack_fault(struct ctx_reg *reg, uint32_t error_code);
LOCAL void handle_gfp(struct ctx_reg *reg, uint32_t error_code);


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
void (*interrupt_handler_func[NUM_IDT_DESCS])(struct ctx_reg*) = {
	[INT_DIV_0]		= handle_division_by_0,
	[INT_DEBUG]		= handle_debug,
	[INT_NMI]		= handle_nmi,
	[INT_BREAKPOINT]	= handle_breakpoint,
	[INT_INT0]		= handle_int0,
	[INT_BOUND]		= handle_bound,
	[INT_INVALID_OPCODE]	= handle_invalid_opcode,
	[INT_CP_NO_AVAILABLE]	= handle_cp_not_available,
	[INT_DOUBLE_FAULT]	= NULL,
	[INT_CP_SEG_OVERRUN]	= handle_cp_seg_overrun,
	[INT_TSS_ERROR]		= NULL,
	[INT_SEG_NOT_PRESENT]	= NULL,
	[INT_STACK_FAULT]	= NULL,
	[INT_GPF]		= NULL,
	[INT_PAGE_FAULT]	= NULL,
	[INT_RESERVED]		= NULL,
	[INT_FP_ERROR]		= handle_math_fault,
	[INT_ALIGN_CHECK]	= handle_alignment_check,
	[INT_MACHINE_CHECK]	= handle_machine_check,
	[INT_SIMD_FP_EXCEPTION]	= handle_simd_fp,
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_default_int_handlers
 Input		:void
 Output		:void
 Return		:void
 Description	:intialize default handlers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void init_default_int_handlers(void)
{
	int i;

	for (i = NUM_EXCEPTION; i < NUM_IDT_DESCS; i++) {
		interrupt_handler_func[i] = NULL;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_int_handler
 Input		:int int_num
 		 < interrupt number to register the handler >
 		 void (*handler)(struct ctx_reg*)
 		 < interrupt handler to register >
 Output		:void
 Return		:ER
 		 < request result >
 Description	:request for registering interrupt handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER
register_int_handler(int int_num, void (*handler)(struct ctx_reg*))
{
	if ((int_num < INT_IRQ0) || (NUM_IDT_DESCS <= int_num)) {
		return(E_PAR);
	}

	if (interrupt_handler_func[int_num]) {
		return(E_BUSY);
	}

	interrupt_handler_func[int_num] = handler;

	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_int_handler
 Input		:int int_num
 		 < interrupt number to unregister the handler >
 Output		:void
 Return		:void
 Description	:request for unregistering interrupt handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void unregister_int_handler(int int_num)
{
	if ((int_num < INT_IRQ0) || (NUM_IDT_DESCS <= int_num)) {
		return;
	}

	interrupt_handler_func[int_num] = NULL;

	return;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:exception_handler_err
 Input		:uint32_t int_num
 		 < interrupt number >
 		 struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code >
 Output		:void
 Return		:void
 Description	:handler for exception with error code
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void exception_handler_err(uint32_t int_num,
			struct ctx_reg *reg, uint32_t error_code)
{
	switch (int_num) {
	case	8:
		handle_double_fault(reg, error_code);
		break;
	case	10:
		handle_tss_error(reg, error_code);
		break;
	case	11:
		handle_segment_not_present(reg, error_code);
		break;
	case	12:
		handle_stack_fault(reg, error_code);
		break;
	case	13:
		handle_gfp(reg, error_code);
		break;
	default:
		vd_printf("unknown interrupt\n");
		break;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:interrupt_handler
 Input		:uint32_t int_num
 		 < interrupt number >
 		 struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:handler for exception/interrupt without error code
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void interrupt_handler(uint32_t int_num, struct ctx_reg *reg)
{
	if (interrupt_handler_func[int_num]) {
		interrupt_handler_func[int_num](reg);
	}
}


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
EXPORT void page_fault_handler(uint32_t int_num, struct ctx_reg *reg,
			uint32_t error_code, unsigned long fault_address)
{
	static int count = 0;

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
----------------------------------------------------------------------------------
	Default exception handlers without error code
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:handle_division_by_0
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(0) for divsion by 0 exception
==================================================================================
*/
LOCAL void handle_division_by_0(struct ctx_reg *reg)
{
	vd_printf("division by 0\n");
}

/*
==================================================================================
 Funtion	:handle_debug
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(1) for debug exception
==================================================================================
*/
LOCAL void handle_debug(struct ctx_reg *reg)
{
	static int count = 0;
	
	if (!count) {
		vd_printf("debug\n");
		count++;
	}
}

/*
==================================================================================
 Funtion	:handle_nmi
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(2) for nmi exception
==================================================================================
*/
LOCAL void handle_nmi(struct ctx_reg *reg)
{
	vd_printf("NMI\n");
}

/*
==================================================================================
 Funtion	:handle_breakpoint
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(3) for breakpoint exception
==================================================================================
*/
LOCAL void handle_breakpoint(struct ctx_reg *reg)
{
	vd_printf("breakpoint\n");
}

/*
==================================================================================
 Funtion	:handle_int0
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(4) for int0 exception
==================================================================================
*/
LOCAL void handle_int0(struct ctx_reg *reg)
{
	vd_printf("int0\n");
}

/*
==================================================================================
 Funtion	:handle_bound
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(5) for bound exception
==================================================================================
*/
LOCAL void handle_bound(struct ctx_reg *reg)
{
	vd_printf("bound\n");
}

/*
==================================================================================
 Funtion	:handle_invalid_opcode
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(6) for invalid opcode exception
==================================================================================
*/
LOCAL void handle_invalid_opcode(struct ctx_reg *reg)
{
	vd_printf("invalid opcode\n");
}

/*
==================================================================================
 Funtion	:handle_cp_not_available
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(7) for coprocessor not available exception
==================================================================================
*/
LOCAL void handle_cp_not_available(struct ctx_reg *reg)
{
	vd_printf("coprocessor not available\n");
}

/*
==================================================================================
 Funtion	:handle_cp_seg_overrun
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(9) for coprocessor segment overrun exception
==================================================================================
*/
LOCAL void handle_cp_seg_overrun(struct ctx_reg *reg)
{
	vd_printf("coprocessor segment overrun\n");
}

/*
==================================================================================
 Funtion	:handle_math_fault
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(16) for floating-point error exception
==================================================================================
*/
LOCAL void handle_math_fault(struct ctx_reg *reg)
{
	vd_printf("floating-point error\n");
}

/*
==================================================================================
 Funtion	:handle_alignment_check
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(17) for alignment check exception
==================================================================================
*/
LOCAL void handle_alignment_check(struct ctx_reg *reg)
{
	vd_printf("alignment check\n");
}

/*
==================================================================================
 Funtion	:handle_machine_check
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(18) for machine check exception
==================================================================================
*/
LOCAL void handle_machine_check(struct ctx_reg *reg)
{
	vd_printf("machine check\n");
}

/*
==================================================================================
 Funtion	:handle_simd_fp
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(19) for simd floating-point exception
==================================================================================
*/
LOCAL void handle_simd_fp(struct ctx_reg *reg)
{
	vd_printf("SIMD floating-point\n");
}

/*
----------------------------------------------------------------------------------
	Default exception handlers with error code
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:handle_double_fault
 Input		:struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code>
 Output		:void
 Return		:void
 Description	:default handler(8) for double fault exception
==================================================================================
*/
LOCAL void handle_double_fault(struct ctx_reg *reg, uint32_t error_code)
{
	vd_printf("error code:%u\n", error_code);
	vd_printf("double fault[0x%08X]\n", error_code);
}

/*
==================================================================================
 Funtion	:handle_tss_error
 Input		:struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code>
 Output		:void
 Return		:void
 Description	:default handler(10) for tss error exception
==================================================================================
*/
LOCAL void handle_tss_error(struct ctx_reg *reg, uint32_t error_code)
{
	static int tss_error = 0;
	
	if (tss_error++) {
		vd_printf("error code:%u\n", error_code);
		vd_printf("tss error[0x%08X]\n", error_code);
		for (;;);
	}
}

/*
==================================================================================
 Funtion	:handle_segment_not_present
 Input		:struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code>
 Output		:void
 Return		:void
 Description	:default handler(11) for segment not present exception
==================================================================================
*/
LOCAL void handle_segment_not_present(struct ctx_reg *reg, uint32_t error_code)
{
	vd_printf("error code:%u\n", error_code);
	vd_printf("segment not present[0x%08X]\n", error_code);
}

/*
==================================================================================
 Funtion	:handle_stack_fault
 Input		:struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code>
 Output		:void
 Return		:void
 Description	:default handler(12) for stack fault exception
==================================================================================
*/
LOCAL void handle_stack_fault(struct ctx_reg *reg, uint32_t error_code)
{
	vd_printf("error code:%u\n", error_code);
	vd_printf("stack fault[0x%08X]\n", error_code);
}

/*
==================================================================================
 Funtion	:handle_gfp
 Input		:struct ctx_reg *reg
 		 < context register information >
 		 uint32_t error_code
 		 < error code>
 Output		:void
 Return		:void
 Description	:default handler(13) for general protection fault exception
==================================================================================
*/
LOCAL void handle_gfp(struct ctx_reg *reg, uint32_t error_code)
{
	static int count = 0;

	if (1 < count) {
		
	} else {
		vd_printf("general protection fault[0x%08X]\n", error_code);
		vd_printf("eax:0x%08X ", reg->eax);
		vd_printf("ebx:0x%08X ", reg->ebx);
		vd_printf("ecx:0x%08X\n", reg->ecx);
		vd_printf("eip:0x%08X ", reg->eip);
		vd_printf("eflags:0x%08X ", reg->eflags);
		vd_printf("cs:0x%08X\n", reg->cs);
		vd_printf("esp:0x%08X ", reg->esp);
		vd_printf("ss:0x%08X\n", reg->ss);

		count++;
		for(;;);
	}
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
