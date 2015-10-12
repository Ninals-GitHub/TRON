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

#include <cpu/x86/cpu_insn.h>
#include <cpu/x86/cpuid.h>
#include <cpu/x86/descriptor.h>

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
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:disint
 Input		:void
 Output		:void
 Return		:saved value of old eflags
		 < uint32_t >

 Description	:disable interrupt wit saving flags
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT uint32_t disint(void)
{
	uint32_t flags;

	ASM (
	"pushfl				\n\t"
	"popl	%[flags]		\n\t"
	"cli				\n\t"
	:[flags]"=m"(flags)
	:
	:);

	return(flags);
	ASM ("nop			\n\t");
}

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
EXPORT uint32_t enaint(uint32_t flags)
{
	uint32_t old_flags;

	ASM (
	"pushfl				\n\t"
	"popl	%[old_flags]		\n\t"
	"pushl	%[flags]		\n\t"
	"popfl				\n\t"
	:[old_flags]"=m"(old_flags)
	:[flags]"m"(flags)
	:);

	return(old_flags);
}

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
EXPORT uint32_t saveFlags(void)
{
	uint32_t flags;
	
	ASM ("pushfl ; pop %0" : "=m" (flags) ::"eax" );
	
	return( flags );
}

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
EXPORT void restoreFlags(uint32_t flags)
{
	
	ASM ("push %0; popfl": :"g" (flags) : "memory", "cc");
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
EXPORT uint32_t getCr0(void)
{
	uint32_t flags;
	
	ASM (
	"movl	%%cr0, %%eax		\n\t"
	"movl	%%eax, %0		\n\t"
	:"=m"(flags)
	:
	:"eax"
	);

	return(flags);
}

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
EXPORT void setCr0(uint32_t flags)
{
	ASM (
	"movl	%0, %%eax		\n\t"
	"movl	%%eax, %%cr0		\n\t"
	:
	:"m"(flags)
	:"eax"
	);
}

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
EXPORT uint32_t getCr2(void)
{
	uint32_t flags;
	
	ASM (
	"movl	%%cr2, %%eax		\n\t"
	"movl	%%eax, %0		\n\t"
	:"=m"(flags)
	:
	:"eax"
	);

	return(flags);
}

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
EXPORT uint32_t getCr3(void)
{
	uint32_t flags;
	
	ASM (
	"movl	%%cr3, %%eax		\n\t"
	"movl	%%eax, %0		\n\t"
	:"=m"(flags)
	:
	:"eax"
	);

	return(flags);
}

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
EXPORT void setCr3(uint32_t flags)
{
	ASM (
	"movl	%0, %%eax		\n\t"
	"movl	%%eax, %%cr3		\n\t"
	:
	:"m"(flags)
	:"eax"
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
EXPORT uint32_t getCr4(void)
{
	uint32_t flags;
	
	ASM (
	"movl	%%cr4, %%eax		\n\t"
	"movl	%%eax, %0		\n\t"
	:"=m"(flags)
	:
	:"eax"
	);

	return(flags);
}

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
EXPORT void setCr4(uint32_t flags)
{
	ASM (
	"movl	%0, %%eax		\n\t"
	"movl	%%eax, %%cr4		\n\t"
	:
	:"m"(flags)
	:"eax"
	);
}

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
EXPORT void loadPdbr(unsigned long new_pde)
{
	ASM (
	"mov %[new_pde], %%cr3	\n\t"
	:
	:[new_pde]"r"(new_pde)
	: );
}

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
EXPORT void invlpg(unsigned long address);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:storePdbr
 Input		:void
 Output		:void
 Return		:unsigned long

 Description	:load pdbr of cr3 register
		 ( for setting page directory entory address )
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT unsigned long storePdbr( void )
{
	unsigned long pdbr;
	
	ASM ( "mov %%cr3, %0\n\t" : "=a"(pdbr) :  );
	
	return( pdbr >> 12 );
		
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:pagingOn
 Input		:void
 Output		:void
 Return		:void

 Description	:set page enable bit
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void pagingOn( void )
{
	ASM ( "cli			\n\t" );
	ASM ( "mov %cr0, %eax		\n\t" );
	ASM ( "OR $0x80000000, %%eax	\n\t":::"cc" );
	ASM ( "mov %eax, %cr0		\n\t" );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:pagingOff
 Input		:void
 Output		:void
 Return		:void

 Description	:unset page enable bit
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void pagingOff( void )
{
	ASM ( "mov %cr0, %eax		\n\t" );
	ASM ( "and $0x7FFFFFFF, %%eax	\n\t":::"cc" );
	ASM ( "mov %eax, %cr0		\n\t" );
}

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
EXPORT void cpuid(uint32_t initial_eax, uint32_t initial_ecx,
			struct cpu_reg32 *regs)
{
	ASM (
	"mov	%[initial_eax], %%eax		\n\t"
	"mov	%[initial_ecx], %%ecx		\n\t"
	"cpuid					\n\t"
	"mov	%%eax, %[regs_eax]		\n\t"
	"mov	%%ebx, %[regs_ebx]		\n\t"
	"mov	%%ecx, %[regs_ecx]		\n\t"
	"mov	%%edx, %[regs_edx]		\n\t"
	:[regs_eax]"=m"(regs->eax), [regs_ebx]"=m"(regs->ebx),
	 [regs_ecx]"=m"(regs->ecx), [regs_edx]"=m"(regs->edx)
	:[initial_eax]"m"(initial_eax) ,[initial_ecx]"m"(initial_ecx)
	:"eax", "ebx", "ecx", "edx"
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:loadGdt
 Input		:unsinged long **addr_gdt
 		 < address of gdt >
 Output		:void
 Return		:void
 Description	:load global segment descriptor table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void loadGdt(unsigned long *addr_gdt)
{
	ASM (
	"movw	%[data_sel], %%ax		\n\t"
	"lgdt	%[addr_gdt]			\n\t"
	"movw	%%ax, %%ds			\n\t"
	"movw	%%ax, %%es			\n\t"
	"movw	%%ax, %%fs			\n\t"
	"movw	%%ax, %%gs			\n\t"
	"movw	%%ax, %%ss			\n\t"
	"jmp	%[code_sel], $_flush_seg	\n"
"_flush_seg:					\t"
	:
	:[addr_gdt]"m"(*addr_gdt),
		[code_sel]"i"(SEG_KERNEL_CS),
		[data_sel]"i"(SEG_KERNEL_DS)
	:"eax"
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setupSSE
 Input		:void
 Output		:void
 Return		:void
 Description	:setup control register for allowing sse instructions
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void setupSSE(void)
{
	uint32_t flags;
	
	/* -------------------------------------------------------------------- */
	/* setup cr0 register							*/
	/* -------------------------------------------------------------------- */
	flags = getCr0();
	flags &= ~CR0_EM;
	flags |= CR0_MP | CR0_NE;
	setCr0(flags);

	/* -------------------------------------------------------------------- */
	/* setup cr4 register							*/
	/* -------------------------------------------------------------------- */
	flags = getCr4();
	flags |= CR4_OSFXSR | CR4_OSXMMEXCPT;
	setCr4(flags);

	ASM ("fninit		\n\t");
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
EXPORT void hlt2(void)
{
	ASM("hlt\n\t");
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
