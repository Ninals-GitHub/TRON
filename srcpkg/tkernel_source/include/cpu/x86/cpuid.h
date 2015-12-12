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

#ifndef	__CPUID_H__
#define	__CPUID_H__

#include <tk/typedef.h>

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
	cpu types
----------------------------------------------------------------------------------
*/
enum cpu_types {
	CPU_TYPE_UNKNOWN	= 0x00000000,
	CPU_TYPE_8086		= 0x00000001,
	CPU_TYPE_80286		= 0x00000002,
	CPU_TYPE_80386		= 0x00000003,
	CPU_TYPE_80486		= 0x00000004,
	CPU_TYPE_80586_LATER	= 0x00000005,
	CPU_TYPE_MASK		= 0x0000FFFF,
	FPU_TYPE_8087		= 0x00010000,
	FPU_TYPE_80287		= 0x00020000,
	FPU_TYPE_80387		= 0x00030000,
	FPU_TYPE_MASK		= 0xFFFF0000,
};

/*
----------------------------------------------------------------------------------
	cache descriptors
----------------------------------------------------------------------------------
*/
#define	CPUINFO_CACHE_DESC	32

/*
----------------------------------------------------------------------------------
	deterministic cache parameter
----------------------------------------------------------------------------------
*/
#define	CPUINFO_CACHE_PARAM	3

struct cache_params {
	uint32_t	max_threads_sharing;
	uint32_t	fully_associative;
	uint32_t	self_initializing_cache_level;
	uint32_t	cache_level;
	uint32_t	cache_type;
	uint32_t	ways_of_associativity;
	uint32_t	physical_line_partitions;
	uint32_t	system_coherency_line_size;
	uint32_t	number_of_sets;
	/* 0 : direct map							*/
	uint32_t	complex_cache_indexing;
	/* 0 : not inclusive of lower cache levels				*/
	uint32_t	lower_cache_leves;
	/* 0 : acts upon all lower level caches
	   1 : not guranteed to act upon lower level caches			*/
	uint32_t	invd_behavior;
	uint32_t	cache_size;
	uint32_t	cache_mask;
};

/*
----------------------------------------------------------------------------------
	monitor information
----------------------------------------------------------------------------------
*/
struct monitor_info {
	/* -------------------------------------------------------------------- */
	/* 05:MONITOR/MWAIT parameters						*/
	/* -------------------------------------------------------------------- */
	uint32_t	min_monitor_line_size;
	uint32_t	max_monitor_line_size;
	uint32_t	interrupt_as_break_event;
	uint32_t	monitor_extensions;
	uint32_t	num_C0;
	uint32_t	num_C1;
	uint32_t	num_C2;
	uint32_t	num_C3;
	uint32_t	num_C4;
	/* -------------------------------------------------------------------- */
	/* 06:MONITOR/MWAIT parameters						*/
	/* -------------------------------------------------------------------- */
	uint32_t	ds_pm_capability;	// higher 16 bits : eax flags
						// lower 16 bits : ecx flags
	uint32_t	interrupt_threshold;
	/* -------------------------------------------------------------------- */
	/* 0A:architectural performance monitor features			*/
	/* -------------------------------------------------------------------- */
	int32_t		version_id;
	uint32_t	num_gppmc;	// number of general-purpose perfomance
					// monitoring counters per logical proc
	uint32_t	monitoring_counter_bits;
	uint32_t	events_length;
	uint32_t	features;
	uint32_t	fixed_counter_bits;
	uint32_t	num_fixed_counter;
};

/* ds_pm_capability [eax]							*/
#define	CPUINFO_CAP_PTM			MAKE_BIT32(6+16)
#define	CPUINFO_CAP_ECMD		MAKE_BIT32(5+16)
#define	CPUINFO_CAP_PLN			MAKE_BIT32(4+16)
#define	CPUINFO_CAP_ARAT		MAKE_BIT32(2+16)
#define	CPUINFO_CAP_TURBO_BOOST		MAKE_BIT32(1+16)
#define	CPUINFO_CAP_DTS			MAKE_BIT32(0+16)
/* ds_pm_capability [ecx]							*/
#define	CPUINFO_CAP_ENERGY_PERF_BIAS	MAKE_BIT32(3)
#define	CPUINFO_CAP_ACNT2		MAKE_BIT32(1)
#define	CPUINFO_CAP_HCF			MAKE_BIT32(0)



/*
----------------------------------------------------------------------------------
	core level processor topology
----------------------------------------------------------------------------------
*/
#define	CPUINFO_LEAF_LEVEL		32

struct topology {
	uint32_t	apic_id_shift;
	uint32_t	num_logical_proc;
	uint32_t	level_type;
	uint32_t	level_number;
	uint32_t	extended_apic_id;
};

/*
----------------------------------------------------------------------------------
	processor extended state enumeration
----------------------------------------------------------------------------------
*/
struct xfeature_state {
	uint32_t	size;
	uint32_t	offset;
};

/*
----------------------------------------------------------------------------------
	cpu information
----------------------------------------------------------------------------------
*/
struct cpu_info {
	/* -------------------------------------------------------------------- */
	/* old cpu type information						*/
	/* -------------------------------------------------------------------- */
	enum cpu_types	cpu_type;
	/* -------------------------------------------------------------------- */
	/* 00:vendor-id and largest standard function				*/
	/* -------------------------------------------------------------------- */
	char		vendor_id[16];		//null termination
	uint32_t	largest_std_func;
	/* -------------------------------------------------------------------- */
	/* 01:feature information						*/
	/* -------------------------------------------------------------------- */
	uint32_t	proc_signature;
	uint32_t	type;
	uint32_t	family_code;
	uint32_t	model_number;
	uint32_t	stepping_id;
	uint32_t	apic_id;
	uint32_t	max_logical_proc;
	uint32_t	clflush_line_size;
	uint32_t	brand_index;		// 0 means not supported
	uint32_t	feature_ecx;
	uint32_t	feature_edx;
	uint32_t	fast_systemcall;	// 0 : not supported
	uint32_t	hyper_threading;	// 0 : not hyper threading
	uint32_t	psn;			// 0 : serial number not supported
	/* -------------------------------------------------------------------- */
	/* 02:cache descriptors							*/
	/* -------------------------------------------------------------------- */
	uint32_t	cache_desc_rec;
	uint8_t		cache_desc[CPUINFO_CACHE_DESC];
	/* -------------------------------------------------------------------- */
	/* 03:processor serial number						*/
	/* -------------------------------------------------------------------- */
	uint32_t	proc_serialnumber_middle;
	uint32_t	proc_serialnumber_least;
	/* -------------------------------------------------------------------- */
	/* 04:deterministic cache parameters					*/
	/* -------------------------------------------------------------------- */
	uint32_t	max_cores;
	struct cache_params cache_params[CPUINFO_CACHE_PARAM];
	/* -------------------------------------------------------------------- */
	/* 05:MONITOR/MWAIT parameters						*/
	/* 06:MONITOR/MWAIT parameters						*/
	/* -------------------------------------------------------------------- */
	struct monitor_info monitor_info;
	/* -------------------------------------------------------------------- */
	/* 07:structured extended feature flags enumeration			*/
	/* -------------------------------------------------------------------- */
	uint32_t	max_sub_leaf;
	uint32_t	extended_feature;
	/* -------------------------------------------------------------------- */
	/* 09:direct cache access parameters					*/
	/* -------------------------------------------------------------------- */
	uint32_t	platform_dca_cap;
	/* -------------------------------------------------------------------- */
	/* 0B:x2apic features/processor topology				*/
	/* -------------------------------------------------------------------- */
	uint32_t	leaf_level;
	struct topology	topology[CPUINFO_LEAF_LEVEL];
	/* -------------------------------------------------------------------- */
	/* 0D:XSAVE features							*/
	/* -------------------------------------------------------------------- */
	uint32_t	xcr0_lower32;
	uint32_t	xcr0_upper32;
	uint32_t	max_xcr0_size;
	uint32_t	max_xsave_size;
	uint32_t	xsaveopt;
	uint32_t	xfeature_len;
	struct xfeature_state xfeature_state[CPUINFO_LEAF_LEVEL];
	/* -------------------------------------------------------------------- */
	/* 80000000:largest extended function					*/
	/* -------------------------------------------------------------------- */
	uint32_t	largest_xfunc;
	/* -------------------------------------------------------------------- */
	/* 80000001:extended feature bits					*/
	/* -------------------------------------------------------------------- */
	uint32_t	lahf;
	uint32_t	xfeatures;
	/* -------------------------------------------------------------------- */
	/* 80000002-4:brand strings						*/
	/* -------------------------------------------------------------------- */
	char		brand_string[48];
	/* -------------------------------------------------------------------- */
	/* 80000006:L2 cache details						*/
	/* -------------------------------------------------------------------- */
	uint32_t	l2_cache_size;		// uinit is 1-KB
	uint32_t	l2_cache_associativity;
	uint32_t	l2_cache_line_size;	// uint is bytes
	/* -------------------------------------------------------------------- */
	/* 80000007:advanced power management					*/
	/* -------------------------------------------------------------------- */
	uint32_t	tsc_invariance;
	/* -------------------------------------------------------------------- */
	/* 80000008:virtual and physical address sizes				*/
	/* -------------------------------------------------------------------- */
	uint32_t	va_size;
	uint32_t	pa_size;
	/* -------------------------------------------------------------------- */
	/* denormals are zeros							*/
	/* -------------------------------------------------------------------- */
	uint32_t	denormals_are_zeros;
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
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initCpuInformation
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize x86 cpu information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void initCpuInformation(void);

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
IMPORT struct cpu_info* getCpuInfo(void);

#endif	// __CPUID_H__
