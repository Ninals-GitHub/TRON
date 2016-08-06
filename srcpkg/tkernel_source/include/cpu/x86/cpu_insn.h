/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	cpu_insn.h (x86)
 *	x86 Dependent Operation
 */

#ifndef _CPU_INSN_
#define _CPU_INSN_

#include <stdint.h>
#include <compiler.h>
#include <tstdlib/bitop.h>
#include <sys/sysinfo.h>
#include <cpu/x86/cpuid.h>
#include <cpu/x86/descriptor.h>
#include <cpu/x86/register.h>

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
	general registers
----------------------------------------------------------------------------------
*/
struct cpu_reg32 {
	uint32_t	eax;
	uint32_t	ebx;
	uint32_t	ecx;
	uint32_t	edx;
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

/* ------------------------------------------------------------------------ */
/*
 *	Control register operation
 */
/*
 * TLB disable
 */
Inline void PurgeTLB( void )
{
	ASM (
	"mov %%cr3, %%eax		\n\t"
	"mov %%eax, %%cr3		\n\t"
	:
	:
	:"eax");
}

/* ------------------------------------------------------------------------ */
/*
 *	EIT-related
 */

/*
 * Vector numbers used by the T-Monitor
 */
#define VECNO_DEFAULT	EIT_DEFAULT	/* default handler */
#define VECNO_IDEBUG	EIT_IDEBUG	/* debug abort instruction */
#define VECNO_DDEBUG	EIT_DDEBUG	/* debug abort data */
#define VECNO_MONITOR	SWI_MONITOR	/* monitor service call */
#define VECNO_ABORTSW	EIT_GPIO(8)	/* abort switch */

#define VECNO_GIO0	EIT_IRQ(50)	/* Generic handler for GPIO interrupt */
#define VECNO_GIO1	EIT_IRQ(51)
#define VECNO_GIO2	EIT_IRQ(52)
#define VECNO_GIO3	EIT_IRQ(53)
#define VECNO_GIO4	EIT_IRQ(79)
#define VECNO_GIO5	EIT_IRQ(80)
#define VECNO_GIO6	EIT_IRQ(26)
#define VECNO_GIO7	EIT_IRQ(27)

/*
 * To save monitor exception handler
 */
typedef struct monhdr {
	FP	default_hdr;		/* default handler */
	FP	idebug_hdr;		/* debug abort instruction */
	FP	ddebug_hdr;		/* debug abort data */
	FP	monitor_hdr;		/* monitor service call */
	FP	abortsw_hdr;		/* abort swiitch  */
	FP	gio_hdr[8];		/* Generic handler for GPIO interrupt */
} MONHDR;

/* For saving monitor exception handler */
//IMPORT MONHDR	SaveMonHdr;

/*
 * If it is the task-independent part, TRUE
 */
Inline BOOL isTaskIndependent( void )
{
	return ( SCInfo.taskindp > 0 )? TRUE: FALSE;
}

/*
 * Move to/Restore task independent part
 */
Inline void EnterTaskIndependent( void )
{
	SCInfo.taskindp++;
}
Inline void LeaveTaskIndependent( void )
{
	SCInfo.taskindp--;
}

/* ------------------------------------------------------------------------ */
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion     :hlt
	Input       :void
	Output      :void
	Return      :void

	Description :halt
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define hlt( ) ({ __asm__ __volatile__ ( "hlt" ); })

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion     :enableInt
	Input       :void
	Output      :void
	Return      :void

	Description :enable interrupt
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define enableInt( ) ({ __asm__ __volatile__ ( "sti" ); })


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion     :disableInt
	Input       :void
	Output      :void
	Return      :void

	Description :disable interrupt
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define disableInt( ) ({ __asm__ __volatile__ ( "cli" ); })

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:disint
 Input		:void
 Output		:void
 Return		:saved value of old eflags
		 < uint32_t >

 Description	:disable interrupt wit saves flags
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t disint(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:enaint
 Input		:uint32_t flags
 Output		:void
 Return		:saved value of old eflags
		 < uint32_t >

 Description	:enable interrupt with saving flags.
 		 actually the function restore the flags
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t enaint(uint32_t flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:saveFlags
 Input		:void
 Output		:void
 Return		:value of eflags
		 < uint32_t >

 Description	:save eflags value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t saveFlags(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:restoreFlags
 Input		:uint32_t flags
		 < flags to load to efalgs register >
 Output		:void
 Return		:void

 Description	:load the flags to eflags
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void restoreFlags(uint32_t flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:irqs_disabled
 Input		:void
 Output		:void
 Return		:int
 		 < boolean result >
 Description	:check whether irqs are disabled or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int irqs_disabled(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_gs
 Input		:uint16_t value
 		 < value to set to gs register >
 Output		:void
 Return		:void
 Description	:set a value to gs register
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void set_gs(uint16_t value)
{
	ASM (
	"movw %[gs_value], %%gs			\n\t"
	:
	:[gs_value]"m"(value)
	:
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_gs
 Input		:void
 Output		:void
 Return		:uint16_t
 		 < value of gs register >
 Description	:get a value of gs register
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE uint16_t get_gs(void)
{
	uint16_t value;
	
	ASM (
	"movw %%gs, %[gs_value]			\n\t"
	:[gs_value]"=m"(value)
	:
	:
	);
	
	return(value);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_fs
 Input		:uint16_t value
 		 < value to set to fs register >
 Output		:void
 Return		:void
 Description	:set a value to fs register
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void set_fs(uint16_t value)
{
	ASM (
	"movw %[fs_value], %%fs			\n\t"
	:
	:[fs_value]"m"(value)
	:
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_fs
 Input		:void
 Output		:void
 Return		:uint16_t
 		 < value of fs register >
 Description	:get a value of fs register
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE uint16_t get_fs(void)
{
	uint16_t value;
	
	ASM (
	"movw %%fs, %[fs_value]			\n\t"
	:[fs_value]"=m"(value)
	:
	:
	);
	
	return(value);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getEsp
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < value of esp >

 Description	:get value of esp
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE uint32_t getEsp(void)
{
	uint32_t value;
	ASM (
	"movl %%esp, %[value]		\n\t"
	:[value]"=m"(value)
	:
	:);

	return(value);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setEsp
 Input		:uint32_t value
 		 < value to set to esp >
 Output		:void
 Return		:void

 Description	:set value tof esp
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void setEsp(uint32_t value)
{
	ASM (
	"movl %[value], %%esp		\n\t"
	:[value]"=m"(value)
	:
	:);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getCr0
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < flags in cr0 >

 Description	:get value of cr0
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t getCr0(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setCr0
 Input		:uint32_t flags
 		 < flags to set to cr0 >
 Output		:void
 Return		:void
 Description	:set value to cr0
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setCr0(uint32_t flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getCr2
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < flags in cr2 >

 Description	:get value of cr2
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t getCr2(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getCr3
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < flags in cr3 >

 Description	:get value of cr3
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t getCr3(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setCr3
 Input		:uint32_t flags
 		 < flags to set to cr3 >
 Output		:void
 Return		:void
 Description	:set value to cr3
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setCr3(uint32_t flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:flush_all_tlb
 Input		:void
 Output		:void
 Return		:void
 Description	:read from cr3 and set the value to cr3
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void flush_all_tlb(void)
{
	setCr3(getCr3());
	ASM (
	"jmp	flush_tlb_after_load_pdbr	\n\t"
	"flush_tlb_after_load_pdbr:"
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getCr4
 Input		:void
 Output		:void
 Return		:uint32_t
 		 < flags in cr4 >

 Description	:get value of cr4
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT uint32_t getCr4(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setCr4
 Input		:uint32_t flags
 		 < flags to set to cr4 >
 Output		:void
 Return		:void
 Description	:set value to cr4
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setCr4(uint32_t flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:loadPdbr
 Input		:unsigned long *new_pde
		 < physical address of page directory entry >
 Output		:void
 Return		:void

 Description	:load pdbr of cr3 registe( for setting page directory entory 
		 address )
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void loadPdbr(unsigned long new_pde);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:invlpg
 Input		:unsigned long address
		 < virtual address to flush >
 Output		:void
 Return		:void

 Description :flush page
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void invlpg(unsigned long address);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:store_pdbr
 Input		:void
 Output		:void
 Return		:unsigned long

 Description	:load pdbr of cr3 register
		 ( for setting page directory entory address )
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT unsigned long storePdbr( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:pagingOn
 Input		:void
 Output		:void
 Return		:void

 Description	:set page enable bit
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void pagingOn( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:pagingOff
 Input		:void
 Output		:void
 Return		:void

 Description	:unset page enable bit
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void pagingOff( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:cpuid
 Input		:uint32_t initial_eax
 		 < argument of cpuid >
 		 uint32_t initial_ecx
 		 < argument of cpuid >
 		 struct cpu_reg32 *regs
 		 < regs resulted in >
 Output		:struct cpu_reg32 *regs
 		 < cpuid instruction result are stored >
 Return		:void
 Description	:execute cpuid instruction
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void cpuid(uint32_t initial_eax, uint32_t initial_ecx,
			struct cpu_reg32 *regs);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:loadGdt
 Input		:unsinged long *addr_gdt
 		 < address of gdt >
 Output		:void
 Return		:void
 Description	:load global segment descriptor table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void loadGdt(unsigned long *addr_gdt);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setupSSE
 Input		:void
 Output		:void
 Return		:void
 Description	:setup control register for allowing sse instructions
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setupSSE(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:out_b
 Input		:uint16_t port
 		 < port address >
 		 uint8_t data
 		 < data to out >
 Output		:void
 Return		:void
 Description	:output a byte data to a port
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void out_b(uint16_t port, uint8_t data)
{
	ASM (
	"outb  %b[data], %w[port]		\n\t"
	:
	:[data]"a"(data), [port]"Nd"(port)
	:);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:out_h
 Input		:uint16_t port
 		 < port address >
 		 uint16_t data
 		 < data to out >
 Output		:void
 Return		:void
 Description	:output a half-word data to a port
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void out_h(uint16_t port, uint16_t data)
{
	ASM (
	"outw  %w[data], %w[port]		\n\t"
	:
	:[data]"a"(data), [port]"d"(port)
	:);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:out_h
 Input		:uint16_t port
 		 < port address >
 		 uint32_t data
 		 < data to out >
 Output		:void
 Return		:void
 Description	:output a word data to a port
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void out_w(uint16_t port, uint32_t data)
{
	ASM (
	"outl  %[data], %w[port]		\n\t"
	:
	:[data]"a"(data), [port]"d"(port)
	:);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:in_b
 Input		:uint16_t port
 		 < port address >
 Output		:void
 Return		:uint8_t data
 		 < input data >
 Description	:input a byte data from a port
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE uint8_t in_b(uint16_t port)
{
	uint8_t data;
	
	ASM (
	"inb  %w[port], %b[data]		\n\t"
	:[data]"=a"(data)
	:[port]"Nd"(port)
	:);

	return(data);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:in_h
 Input		:uint16_t port
 		 < port address >
 Output		:void
 Return		:uint16_t data
 		 < input data >
 Description	:input a half-word data from a port
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE uint16_t in_h(uint16_t port)
{
	uint16_t data;
	
	ASM (
	"inw  %w[port], %w[data]		\n\t"
	:[data]"=a"(data)
	:[port]"d"(port)
	:);

	return(data);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:in_w
 Input		:uint16_t port
 		 < port address >
 Output		:void
 Return		:uint32_t data
 		 < input data >
 Description	:input a word data from a port
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE uint32_t in_w(uint16_t port)
{
	uint32_t data;
	
	ASM (
	"inl  %w[port], %[data]		\n\t"
	:[data]"=a"(data)
	:[port]"d"(port)
	:);

	return(data);
}

#endif /* _CPU_INSN_ */
