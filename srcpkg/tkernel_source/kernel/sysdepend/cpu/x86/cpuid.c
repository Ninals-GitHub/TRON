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
#include <libstr.h>
#include <tstdlib/bitop.h>

#include <debug/vdebug.h>

#include <cpu/x86/cpuid.h>
#include <cpu/x86/cpu_insn.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int check_8086(void);
LOCAL int check_80286(void);
LOCAL int check_80386(void);
LOCAL int check_80486(void);
LOCAL int check_fpu(void);

LOCAL void cpuid_00(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_01(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_02(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_03(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_04(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_05(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_06(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_07(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_09(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_0A(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_0B(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_0D(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_80000000(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_80000001(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_80000006(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_80000007(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void cpuid_80000008(struct cpu_reg32 *regs, struct cpu_info *info);
LOCAL void detectDenormalsAreZeros(struct cpu_info *info);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	Standard CPUID Functions
----------------------------------------------------------------------------------
*/
/* vendor-id and largest standard function					*/
#define	CPUID_VENDOR_ID		0x00000000
/* feature information								*/
#define	CPUID_FEATURE_INFO	0x00000001
/* cache descriptors								*/
#define	CPUID_CACHE_DESC	0x00000002
/* processor serial number							*/
#define	CPUID_SERIAL_NUMBER	0x00000003
/* deterministic cache parameters						*/
#define	CPUID_CACHE_PARAM	0x00000004
/* MONITOR/MWAIT parameters							*/
#define	CPUID_MONITOR_MWAIT	0x00000005
/* digital thermal sensor and power management parameters			*/
#define	CPUID_TSENSOR_POWER	0x00000006
/* structured extended feature flags enumeration				*/
#define	CPUID_EXTENDED_FEAT	0x00000007
/* reserved
			*/
#define	CPUID_RESERVED_1	0x00000008
/* direct cache access parameters						*/
#define	CPUID_DCA_PARAM		0x00000009
/* architectural performance monitor features					*/
#define	CPUID_APM_FEAT		0x0000000A
/* x2apic features/processor topology						*/
#define	CPUID_APIC_P_TOPOLOGY	0x0000000B
/* reserved									*/
#define	CPUID_RESERVED_2	0x0000000C
/* XSAVE features								*/
#define	CPUID_XSAVE_FEAT	0x0000000D

/*
----------------------------------------------------------------------------------
	Extended CPUID Functions
----------------------------------------------------------------------------------
*/
/* largest extended function							*/
#define	EXCPUID_LARGEST_FUNC	0x80000000
/* extended features								*/
#define	EXCPUID_EXTENDED_FEAT	0x80000001
/* processor brand string							*/
#define	EXCPUID_PROC_BRAND_1	0x80000002
#define	EXCPUID_PROC_BRAND_2	0x80000003
#define	EXCPUID_PROC_BRAND_3	0x80000004
/* reserved									*/
#define	EXCPUID_RESERVED_3	0x80000005
/* extended L2 cache features							*/
#define	EXCPUID_L2_CACHE_FEAT	0x80000006
/* advanced power management							*/
#define	EXCPUID_APM		0x80000007
/* virtual and physical address sizes						*/
#define	EXCPUID_ADDRESS_SIZE	0x80000008



/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct cpu_info cpu_info;

/*
----------------------------------------------------------------------------------
	vendor id
----------------------------------------------------------------------------------
*/
const char intel_id[] = "GenuineIntel";


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initCpuInformation
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize x86/x64 cpu information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void initCpuInformation(void)
{
	struct cpu_reg32 regs;

	/* -------------------------------------------------------------------- */
	/* check old cpu and cpuid support					*/
	/* -------------------------------------------------------------------- */
	disableInt();

	if (!check_8086()) {
		vd_printf("8086 processor\n");
		cpu_info.cpu_type = CPU_TYPE_8086 | FPU_TYPE_8087;
		return;
	}

	if (!check_80286()) {
		vd_printf("80286 processor\n");
		cpu_info.cpu_type = CPU_TYPE_80286 | FPU_TYPE_8087;
		return;
	}

	if (!check_80386()) {
		vd_printf("80386 processor\n");
		cpu_info.cpu_type = CPU_TYPE_80386;

		if (!check_fpu()) {
			cpu_info.cpu_type |= FPU_TYPE_80387;
		} else {
			cpu_info.cpu_type |= FPU_TYPE_80287;
		}

		return;
	}
	
	if (!check_80486()) {
		vd_printf("80486 processor\n");
		cpu_info.cpu_type = CPU_TYPE_80486;
		return;
	} else {
		vd_printf("80586 or later\n");
		cpu_info.cpu_type = CPU_TYPE_80586_LATER;
	}
	
	/* -------------------------------------------------------------------- */
	/* execute cpuid instruction						*/
	/* -------------------------------------------------------------------- */
	cpuid_00(&regs, &cpu_info);

	cpuid_01(&regs, &cpu_info);
	cpuid_02(&regs, &cpu_info);
	cpuid_03(&regs, &cpu_info);
	cpuid_04(&regs, &cpu_info);
	cpuid_05(&regs, &cpu_info);
	cpuid_06(&regs, &cpu_info);
	cpuid_07(&regs, &cpu_info);
	cpuid_09(&regs, &cpu_info);
	cpuid_0A(&regs, &cpu_info);
	cpuid_0B(&regs, &cpu_info);
	cpuid_0D(&regs, &cpu_info);

	cpuid_80000000(&regs, &cpu_info);

	cpuid_80000001(&regs, &cpu_info);
	

	cpuid_80000006(&regs, &cpu_info);
	cpuid_80000007(&regs, &cpu_info);
	cpuid_80000008(&regs, &cpu_info);

	detectDenormalsAreZeros(&cpu_info);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getCpuInfo
 Input		:void
 Output		:void
 Return		:struct cpu_info*
 		 < cpu information >
 Description	:get x86/x64 cpu information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct cpu_info* getCpuInfo(void)
{
	return(&cpu_info);
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
 Funtion	:check_8086
 Input		:void
 Output		:void
 Return		:int
 		 < result 0 : may be 8086/8088, 1 : not 8086/8088 >
 Description	:check if a processor is 8086/8088
==================================================================================
*/
LOCAL int check_8086(void)
{
	int ret = 0;
	
	ASM(
	"pushf			\n\t"	// get flags,
	"pop	%%ax		\n\t"
	"mov	%%ax, %%cx	\n\t"	// save to cx
	"and	$0x0FFF, %%ax	\n\t"	// edit flags
	"push	%%ax		\n\t"
	"popf			\n\t"	// update flags
	"pushf			\n\t"	// get flags
	"pop	%%ax		\n\t"
	"and	$0xF000, %%ax	\n\t"
	"cmp	$0xF000, %%ax	\n\t"
	"mov	$0x0000, %0	\n\t"
	"je	end_check_8086	\n\t"	// if bits 12-15 are modified then 8086
	"mov	$0x0001, %0 	\n"	// not 8086/8088
"end_check_8086:\t"
	"push	%%cx		\n\t"	// restore flags
	"popf			\n\t"
	:"=g"(ret)
	:
	:"eax", "ecx");
	return(ret);
}

/*
==================================================================================
 Funtion	:check_80286
 Input		:void
 Output		:void
 Return		:int
 		 < result 0 : 80286, 1 : not 80286 >
 Description	:check if a processor is 80286
==================================================================================
*/
LOCAL int check_80286(void)
{
	int ret = 0;
	
	ASM(
	"pushf			\n\t"	// get flags,
	"pop	%%ax		\n\t"
	"mov	%%ax, %%cx	\n\t"	// save to cx
	"or	$0xF000, %%ax 	\n\t"	// edit flags
	"push	%%ax		\n\t"
	"popf			\n\t"	// update flags
	"pushf			\n\t"	// get flags
	"pop	%%ax		\n\t"
	"and	$0xF000, %%ax 	\n\t"
	"jz	end_check_80286	\n"
	"mov	$0x0001, %0	\n"	// if bits 12-15 are cleared then not 80286
"end_check_80286:\t"
	"push	%%cx		\n\t"	// restore flags
	"popf			\n\t"
	:"=g"(ret)
	:
	:"eax", "eax");
	return(ret);
}

/*
==================================================================================
 Funtion	:check_80386
 Input		:void
 Output		:void
 Return		:int
 		 < result 0 : 80386, 1 : not 80386 >
 Description	:check if a processor is 80386
==================================================================================
*/
LOCAL int check_80386(void)
{
	int ret = 0;
	
	ASM(
	"pushfl			\n\t"	// get eflags,
	"pop	%%eax		\n\t"
	"mov	%%eax, %%ecx	\n\t"	// save to ecx
	"mov	%%ax, %%dx	\n\t"
	"shrl	$16, %%eax	\n\t"	// any of lower flags should be saved
	"xor	$0x0004, %%ax	\n\t"
	"shll	$16, %%eax	\n\t"
	"mov	%%dx, %%ax	\n\t"
	"push	%%eax		\n\t"
	"popfl			\n\t"	// update eflags
	"pushfl			\n\t"	// get eflags
	"pop	%%eax		\n\t"
	"xor	%%eax, %%ecx	\n\t"
	"jz	end_check_80386	\n"	// cannot toggle ac flag if 80386
	"mov	$0x0001, %0	\n"	// not 80386
"end_check_80386:		\t"
	"push	%%ecx		\n\t"	// restore flags
	"popfl			\n\t"
	:"=g"(ret)
	:
	:"eax", "ecx", "edx");
	return(ret);
}

/*
==================================================================================
 Funtion	:check_80486
 Input		:void
 Output		:void
 Return		:int
 		 < result 0 : 80486, 1 : not 80486 >
 Description	:check if a processor is 80486 (or cpuid supported)
==================================================================================
*/
LOCAL int check_80486(void)
{
	int ret = 0;// edit eflags (ID bit)
	
	ASM(
	"pushfl			\n\t"	// get eflags,
	"pop	%%eax		\n\t"
	"mov	%%eax, %%ecx	\n\t"	// save to ecx
	"mov	%%ax, %%dx	\n\t"	// any of lower flags should be reserved
	"shrl	$16, %%eax	\n\t"
	"xor	$0x0020, %%ax	\n\t"
	"shll	$16, %%eax	\n\t"
	"mov	%%dx, %%ax	\n\t"
	"push	%%eax		\n\t"
	"popfl			\n\t"	// update eflags
	"pushfl			\n\t"	// get eflags
	"pop	%%eax		\n\t"
	"xor	%%ecx, %%eax	\n\t"
	"jz	end_check_80486	\n\t"	// cannot toggle id flag if 80486
	"mov	$0x0001, %0	\n"	// not 80486
"end_check_80486:\t"
	"push	%%ecx		\n\t"	// restore flags
	"popfl			\n\t"
	:"=g"(ret)
	:
	:"eax", "ecx");
	return(ret);
}

/*
==================================================================================
 Funtion	:check_fpu
 Input		:void
 Output		:void
 Return		:int
 		 < result 0 : 80387, 1 : 80287 >
 Description	:check if a processor is 80387 for 80836
==================================================================================
*/
LOCAL int check_fpu(void)
{
	int ret = 0;
	
	ASM(
	"fld1			\n\t"
	"fldz			\n\t"	// form infinity
	"fdivp			\n\t"	// 8087/Intel287 NDP say +inf = -inf
	"fld %%st(0)		\n\t"	// form negative infinity
	"fchs			\n\t"	// Intel387 NDP says +inf != -inf
	"fcompp			\n\t"
	"fstsw %%ax		\n\t"
	"sahf			\n\t"
	"mov 0x0000, %0		\n\t"
	"jz end_check_fpu	\n\t"
	"mov 0x0001, %0		\n\t"	// 80827
	
"end_check_fpu:			\t"
	:"=r"(ret)
	:
	:"eax");
	return(ret);
}

/*
==================================================================================
 Funtion	:cpuid_00
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 00
 		 vendor-id and largest standard function
==================================================================================
*/
LOCAL void cpuid_00(struct cpu_reg32 *regs, struct cpu_info *info)
{
	cpuid(CPUID_VENDOR_ID, 0, regs);

	info->largest_std_func = regs->eax;
	
	info->vendor_id[ 0] = bit_value(regs->ebx,  0,  7);
	info->vendor_id[ 1] = bit_value(regs->ebx,  8, 15);
	info->vendor_id[ 2] = bit_value(regs->ebx, 16, 23);
	info->vendor_id[ 3] = bit_value(regs->ebx, 24, 31);

	info->vendor_id[ 4] = bit_value(regs->edx,  0,  7);
	info->vendor_id[ 5] = bit_value(regs->edx,  8, 15);
	info->vendor_id[ 6] = bit_value(regs->edx, 16, 23);
	info->vendor_id[ 7] = bit_value(regs->edx, 24, 31);

	info->vendor_id[ 8] = bit_value(regs->ecx,  0,  7);
	info->vendor_id[ 9] = bit_value(regs->ecx,  8, 15);
	info->vendor_id[10] = bit_value(regs->ecx, 16, 23);
	info->vendor_id[11] = bit_value(regs->ecx, 24, 31);

	info->vendor_id[12] = '\0';
}

/*
==================================================================================
 Funtion	:cpuid_01
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 01
 		 feature information
==================================================================================
*/
LOCAL void cpuid_01(struct cpu_reg32 *regs, struct cpu_info *info)
{
	uint32_t work;
	
	cpuid(CPUID_FEATURE_INFO, 0, regs);

	info->proc_signature = regs->eax;

	/* update cpu type							*/
	info->cpu_type = (enum cpu_types)bit_value(regs->edx, 8, 11);

	/* family code								*/
	work = bit_value(regs->edx, 20, 27);	// extended
	work <<= 4;
	info->family_code = work | bit_value(regs->edx, 8, 11);
	/* model number								*/
	work = bit_value(regs->edx, 16, 19);	// extended
	work <<=4;
	info->model_number = work | bit_value(regs->edx, 4, 7);

	info->type = bit_value(regs->edx, 12, 13);
	info->stepping_id = bit_value(regs->edx, 0, 3);

	info->brand_index = bit_value(regs->ebx, 0, 7);
	info->clflush_line_size = bit_value(regs->ebx, 8, 15);
	info->max_logical_proc = bit_value(regs->ebx, 16, 23);
	info->apic_id = bit_value(regs->ebx, 24, 31);

	info->feature_ecx = regs->ecx;
	info->feature_edx = regs->edx;

	info->hyper_threading = 0;
	if (tstdlib_bittest(&info->feature_edx, 28)) {	// HTT bit
		if (1 < info->max_logical_proc) {
			info->hyper_threading = 1;
		}
	}

	if (tstdlib_bittest(&info->feature_edx, 11)) {	// SEP bit
		if ((info->proc_signature & 0x0FFF3FFF) < 0x00000633 ) {
			info->fast_systemcall = 0;
		} else {
			info->fast_systemcall = 1;
		}
	}

	if (info->model_number == 1) {
		info->fast_systemcall = 0;
	}

	/* prosessor serial number supported					*/
	info->psn = 0;
	if (tstdlib_bittest(&info->feature_edx, 18)) {
		info->psn = 1;
	}
}


/*
==================================================================================
 Funtion	:cpuid_02
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 02
 		 cache descriptors
==================================================================================
*/
LOCAL void cpuid_02(struct cpu_reg32 *regs, struct cpu_info *info)
{
	uint32_t count = 0;
	uint32_t start;
	uint32_t end;
	uint8_t work;
	uint32_t *gregs[5] = {&regs->eax, &regs->ebx, &regs->ecx, &regs->edx, NULL};
	uint32_t **regp;

	int index = 0;
	int i;

	if (info->largest_std_func < CPUID_CACHE_DESC) {
		goto fill_remainders;
	}

	do {
		cpuid(CPUID_CACHE_DESC, 0, regs);
		if (!count) {
			info->cache_desc_rec = bit_value(regs->eax, 0, 7);
			if (!info->cache_desc_rec) {
				info->cache_desc_rec = 1;
			}
		}
		
		/* eax only							 */
		start = 8;
		end = 15;
		/* eax-edx							*/
		for ( regp=gregs ; *regp ; regp++ ) {
			if ((**regp & MAKE_BIT32(31))) {
				/* next						*/
				start = 0;
				end = 7;
				continue;
			}
			for ( ; end < 32 ; start +=8, end += 8 ) {
				work = (uint8_t)(bit_value(**regp, start, end)
					& MAKE_MASK32(0, 7 ));
				
				if (work) {
					info->cache_desc[index++] = work;
					if (sizeof(info->cache_desc) <= index) {
						goto over_flow;
					}
				}
			}
			start = 0;
			end = 7;
			
		}

		count++;
	} while (count < info->cache_desc_rec);

	info->cache_desc_rec = index;

fill_remainders:
	/* remainders are filled with null descriptor				*/
	for (i=index ; i<sizeof(info->cache_desc) ; i++) {
		info->cache_desc[i] = 0x00;
	}

	return;

over_flow:
	info->cache_desc_rec = index;
	return;
}

/*
==================================================================================
 Funtion	:cpuid_03
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 03
 		 processor serial number
==================================================================================
*/
LOCAL void cpuid_03(struct cpu_reg32 *regs, struct cpu_info *info)
{
	if (info->largest_std_func < CPUID_SERIAL_NUMBER) {
		goto fill_out;
	}

	if (info->psn) {
		cpuid(CPUID_SERIAL_NUMBER, 0, regs);

		info->proc_serialnumber_middle = regs->edx;
		info->proc_serialnumber_least = regs->ecx;
	} else {
fill_out:
		info->proc_serialnumber_middle = 0;
		info->proc_serialnumber_least = 0;
	}

}

/*
==================================================================================
 Funtion	:cpuid_04
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 04
 		 deterministic cache parameters
==================================================================================
*/
LOCAL void cpuid_04(struct cpu_reg32 *regs, struct cpu_info *info)
{
	struct cache_params *param = info->cache_params;
	uint32_t ecx;
	uint32_t temp32;
	uint32_t cache_mask_width;

	if (info->largest_std_func < CPUID_CACHE_PARAM) {
		info->max_cores = 0;
		memset((void*)param, 0x00,
			sizeof(struct cache_params) * CPUINFO_CACHE_PARAM );
		return;
	}

	for (ecx = 0 ;
		ecx < sizeof(info->cache_params)/sizeof(struct cache_params) ;
		ecx++) {
		cpuid(CPUID_CACHE_PARAM, ecx, regs);

		if (!ecx) {
			info->max_cores = bit_value(regs->eax, 26, 31) + 1;
		}

		param->cache_type = bit_value(regs->eax, 0, 4);

		if (!param->cache_type) {
			break;
		}

		temp32 = bit_value(regs->eax, 14, 25);
		param->max_threads_sharing = temp32 + 1;
		param->fully_associative =
				(uint32_t)tstdlib_bittest(&regs->eax, 9);
		param->self_initializing_cache_level =
				(uint32_t)tstdlib_bittest(&regs->eax, 8);
		param->cache_level = bit_value(regs->eax, 5, 7);

		param->ways_of_associativity = bit_value(regs->ebx, 22, 31) + 1;
		param->physical_line_partitions = bit_value(regs->ebx, 12, 21) + 1;
		param->system_coherency_line_size = bit_value(regs->ebx, 0, 11 ) + 1;

		param->number_of_sets = regs->ecx + 1;

		param->complex_cache_indexing =
				(uint32_t)tstdlib_bittest(&regs->edx, 2);
		param->lower_cache_leves =
				(uint32_t)tstdlib_bittest(&regs->edx, 1);
		param->invd_behavior =
				(uint32_t)tstdlib_bittest(&regs->edx, 0);

		param->cache_size = param->ways_of_associativity
					* param->physical_line_partitions
					* param->system_coherency_line_size
					* param->number_of_sets;

		for (cache_mask_width = 0 ; temp32++; cache_mask_width++) {
			temp32 >>= 1;
		}

		param->cache_mask = (uint32_t)(-1UL) << temp32;
	}

}

/*
==================================================================================
 Funtion	:cpuid_05
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 05
 		 MONITOR/MWAIT parameters
==================================================================================
*/
LOCAL void cpuid_05(struct cpu_reg32 *regs, struct cpu_info *info)
{
	struct monitor_info *minfo = &info->monitor_info;

	if (info->largest_std_func < CPUID_MONITOR_MWAIT) {
		memset((void*)minfo, 0x00, sizeof(struct monitor_info) );
		return;
	}
	
	cpuid(CPUID_MONITOR_MWAIT, 0, regs);

	minfo->min_monitor_line_size = bit_value(regs->eax, 0, 15);
	minfo->max_monitor_line_size = bit_value(regs->ebx, 0, 15);
	minfo->interrupt_as_break_event = (uint32_t)tstdlib_bittest(&regs->ecx, 1);
	minfo->monitor_extensions = (uint32_t)tstdlib_bittest(&regs->ecx, 0);
	minfo->num_C4 = bit_value(regs->edx, 16, 19);
	minfo->num_C3 = bit_value(regs->edx, 12, 15);
	minfo->num_C2 = bit_value(regs->edx,  8, 11);
	minfo->num_C1 = bit_value(regs->edx,  4,  7);
	minfo->num_C0 = bit_value(regs->edx,  0,  3);
}

/*
==================================================================================
 Funtion	:cpuid_06
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 06
 		 MONITOR/MWAIT parameters
==================================================================================
*/
LOCAL void cpuid_06(struct cpu_reg32 *regs, struct cpu_info *info)
{
	struct monitor_info *minfo = &info->monitor_info;

	if (info->largest_std_func < CPUID_TSENSOR_POWER) {
		return;
	}
	
	cpuid(CPUID_TSENSOR_POWER, 0, regs);

	minfo->ds_pm_capability = bit_value(regs->eax, 0, 15) << 16;
	minfo->ds_pm_capability |= bit_value(regs->ecx, 0, 15);
	minfo->interrupt_threshold = bit_value(regs->ebx, 0, 3);
}

/*
==================================================================================
 Funtion	:cpuid_07
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 07
 		 structured extended feature flags enumeration
==================================================================================
*/
LOCAL void cpuid_07(struct cpu_reg32 *regs, struct cpu_info *info)
{
	if (info->largest_std_func < CPUID_EXTENDED_FEAT) {
		info->max_sub_leaf = 0;
		info->extended_feature = 0;
		return;
	}

	cpuid(CPUID_EXTENDED_FEAT, 0, regs);

	info->max_sub_leaf = regs->eax;
	info->extended_feature = regs->ebx;
}

/*
==================================================================================
 Funtion	:cpuid_09
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 09
 		 direct cache access parameters
==================================================================================
*/
LOCAL void cpuid_09(struct cpu_reg32 *regs, struct cpu_info *info)
{
	if (info->largest_std_func < CPUID_DCA_PARAM) {
		info->platform_dca_cap = 0;
		return;
	}

	cpuid(CPUID_DCA_PARAM, 0, regs);

	info->platform_dca_cap = regs->eax;
}

/*
==================================================================================
 Funtion	:cpuid_0A
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 0A
 		 architectural performance monitor features
==================================================================================
*/
LOCAL void cpuid_0A(struct cpu_reg32 *regs, struct cpu_info *info)
{
	struct monitor_info *minfo = &info->monitor_info;

	if (info->largest_std_func < CPUID_APM_FEAT) {
		minfo->version_id = 0;
		minfo->num_gppmc = 0;
		minfo->monitoring_counter_bits = 0;
		minfo->events_length = 0;
		minfo->features = 0;
		minfo->fixed_counter_bits = 0;
		minfo->num_fixed_counter = 0;
		return;
	}
	
	cpuid(CPUID_APM_FEAT, 0, regs);

	minfo->version_id = bit_value(regs->eax, 0, 7);
	minfo->num_gppmc = bit_value(regs->eax, 8, 15);
	minfo->monitoring_counter_bits = bit_value(regs->eax, 16, 23);
	minfo->events_length = bit_value(regs->eax, 24, 31);

	minfo->features = regs->ebx;

	minfo->fixed_counter_bits = bit_value(regs->edx, 5, 12);
	minfo->num_fixed_counter = bit_value(regs->edx, 0, 4);
}


/*
==================================================================================
 Funtion	:cpuid_0B
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 0B
 		 x2apic features/processor topology
==================================================================================
*/
LOCAL void cpuid_0B(struct cpu_reg32 *regs, struct cpu_info *info)
{
	uint32_t ecx;
	struct topology *topology = info->topology;

	info->leaf_level = 0;
	
	if (info->largest_std_func < CPUID_APIC_P_TOPOLOGY) {
		memset((void*)topology, 0x00,
			sizeof(struct topology) * CPUINFO_LEAF_LEVEL );
		return;
	}

	for (ecx = 0 ; ecx < CPUINFO_LEAF_LEVEL ; ecx++) {
	
		cpuid(CPUID_APIC_P_TOPOLOGY, ecx, regs);

		if (!regs->eax && !regs->ebx) {
			break;
		}
		
		topology->apic_id_shift = bit_value(regs->eax, 0, 4);
		topology->num_logical_proc = bit_value(regs->ebx, 0, 15);
		topology->level_type = bit_value(regs->ecx, 8, 15);
		topology->level_number = bit_value(regs->ecx, 0, 7);
		topology->extended_apic_id = regs->edx;
		
		topology++;

		info->leaf_level++;
	}
}

/*
==================================================================================
 Funtion	:cpuid_0D
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 0D
 		 XSAVE features
==================================================================================
*/
LOCAL void cpuid_0D(struct cpu_reg32 *regs, struct cpu_info *info)
{
	uint32_t ecx = 0;
	struct xfeature_state *xf = info->xfeature_state;

	if (info->largest_std_func < CPUID_XSAVE_FEAT) {
		info->xcr0_lower32 = 0;
		info->xcr0_upper32 = 0;
		info->max_xcr0_size = 0;
		info->max_xsave_size = 0;
		info->xsaveopt = 0;
		info->xfeature_len = 0;
		memset((void*)xf, 0x00,
			sizeof(struct xfeature_state) * CPUINFO_LEAF_LEVEL );
		return;
	}

	cpuid(CPUID_XSAVE_FEAT, ecx++, regs);

	info->xcr0_lower32 = regs->eax;
	info->xcr0_upper32 = regs->edx;
	info->max_xcr0_size = regs->ebx;
	info->max_xsave_size = regs->ecx;

	cpuid(CPUID_XSAVE_FEAT, ecx++, regs);

	info->xsaveopt = (uint32_t)tstdlib_bittest(&regs->eax, 0);

	info->xfeature_len = 0;

	for ( ; ecx < CPUINFO_LEAF_LEVEL ; ecx++) {
	
		cpuid(CPUID_XSAVE_FEAT, ecx, regs);

		if (!regs->eax && !regs->ebx) {
			break;
		}

		xf->size = regs->eax;
		xf->size = regs->ebx;

		info->xfeature_len++;
	}
}


/*
==================================================================================
 Funtion	:cpuid_80000000
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 80000000
 		 largest extended function
==================================================================================
*/
LOCAL void cpuid_80000000(struct cpu_reg32 *regs, struct cpu_info *info)
{
	cpuid(EXCPUID_LARGEST_FUNC, 0, regs);
	info->largest_xfunc = regs->eax;
}

/*
==================================================================================
 Funtion	:cpuid_brandstring
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 80000002, 80000003, 80000004
 		 largest extended function
==================================================================================
*/
LOCAL void cpuid_80000001(struct cpu_reg32 *regs, struct cpu_info *info)
{
	uint32_t *save = (uint32_t*)info->brand_string;
	char *name = info->brand_string;
	char *ptr = name;
	int index;

	if (info->largest_xfunc < EXCPUID_PROC_BRAND_1) {
		memset((void*)name, 0x00, sizeof(info->brand_string));
		return;
	}

	index = 0;
	
	cpuid(EXCPUID_PROC_BRAND_1, 0, regs);

	*(save + index) = regs->eax;
	*(save + index++) = regs->ebx;
	*(save + index++) = regs->ecx;
	*(save + index++) = regs->edx;

	cpuid(EXCPUID_PROC_BRAND_2, 0, regs);

	*(save + index++) = regs->eax;
	*(save + index++) = regs->ebx;
	*(save + index++) = regs->ecx;
	*(save + index++) = regs->edx;

	cpuid(EXCPUID_PROC_BRAND_3, 0, regs);

	*(save + index++) = regs->eax;
	*(save + index++) = regs->ebx;
	*(save + index++) = regs->ecx;
	*(save + index++) = regs->edx;

	/* -------------------------------------------------------------------- */
	/* skip space								*/
	/* -------------------------------------------------------------------- */
	while (*ptr) {
		if (*ptr == ' ') {
			ptr++;
		} else {
			break;
		}
	}

	if (!*ptr) {
		return;
	}

	while (*ptr) {
		*(name++) = *(ptr++);
	}

	*name = '\0';
}

/*
==================================================================================
 Funtion	:cpuid_80000006
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 80000006
 		 L2 cache details
==================================================================================
*/
LOCAL void cpuid_80000006(struct cpu_reg32 *regs, struct cpu_info *info)
{
	if (info->largest_xfunc < EXCPUID_L2_CACHE_FEAT) {
		info->l2_cache_size = 0;
		info->l2_cache_associativity = 0;
		info->l2_cache_line_size = 0;
		return;
	}
	
	cpuid(EXCPUID_L2_CACHE_FEAT, 0, regs);

	info->l2_cache_size = bit_value(regs->ecx, 16, 31);
	info->l2_cache_associativity = bit_value(regs->ecx, 12, 15);
	info->l2_cache_line_size = bit_value(regs->ecx, 0, 7);
}

/*
==================================================================================
 Funtion	:cpuid_80000007
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 80000007
 		 advanced power management
==================================================================================
*/
LOCAL void cpuid_80000007(struct cpu_reg32 *regs, struct cpu_info *info)
{
	if (info->largest_xfunc < EXCPUID_APM) {
		info->tsc_invariance = 0;
		return;
	}
	
	cpuid(EXCPUID_APM, 0, regs);

	info->tsc_invariance = tstdlib_bittest(&regs->edx, 8);
}

/*
==================================================================================
 Funtion	:cpuid_80000008
 Input		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Output		:struct cpu_reg32 *regs
 		 < registers >
 		 struct cpu_info *info
 		 < cpu information >
 Return		:void
 Description	:execute function 80000008
 		 virtual and physical address sizes
==================================================================================
*/
LOCAL void cpuid_80000008(struct cpu_reg32 *regs, struct cpu_info *info)
{
	if (info->largest_xfunc < EXCPUID_ADDRESS_SIZE) {
		info->va_size = 0;
		info->pa_size = 0;
		return;
	}
	
	cpuid(EXCPUID_ADDRESS_SIZE, 0, regs);

	info->va_size = bit_value(regs->eax, 8, 15);
	info->pa_size = bit_value(regs->eax, 0,  7);
}

/*
==================================================================================
 Funtion	:detectDenormalsAreZeros
 Input		:struct cpu_info *info
 		 < cpu information >
 Output		:void
 Return		:void
 
 Description	:detecting denormals are zero
==================================================================================
*/
LOCAL void detectDenormalsAreZeros(struct cpu_info *info)
{
	uint8_t buffer[512+16];
	uint8_t *buf;
	uint32_t mxcsr_mask;

	info->denormals_are_zeros = 0;

	/* fxsr bit								*/
	if (!tstdlib_bittest(&info->feature_edx, 24)) {
		return;
	}
	/* sse/sse2 bit								*/
	if (!tstdlib_bittest(&info->feature_edx, 25) &&
		!tstdlib_bittest(&info->feature_edx, 26)) {
		return;
	}

	/* setup control registers for sse					*/
	setupSSE();

	/* 16 byte align							*/
	buf = (uint8_t*)((unsigned long)buffer & ((-1UL) << 4));
	buf += 16;
	
	/* clear the buffer for fxsave						*/
	memset((void*)buffer, 0x00, sizeof(buffer));

	ASM ("fxsave %0\n\t" :"=m"(*buf):"m"(*buf):"memory");
	
	mxcsr_mask = (uint32_t)(*((uint32_t*)&buf[28]));
	
	if (!mxcsr_mask) {
		return;
	}

	/* bit 6 daz support bit						*/
	if (!tstdlib_bittest(&mxcsr_mask, 6)) {
		return;
	}

	/* supported								*/
	info->denormals_are_zeros = 1;
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
