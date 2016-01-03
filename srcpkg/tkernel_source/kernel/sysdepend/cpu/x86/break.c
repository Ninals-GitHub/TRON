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

#include <typedef.h>
#include <cpu.h>

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
#define	DEBUG_TRAP_INST		0xCC

struct break_point {
	unsigned long	address;
	unsigned char	i;
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct break_point debug_bp;


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:handle_breakpoint
 Input		:struct ctx_reg *reg
 		 < context register information >
 Output		:void
 Return		:void
 Description	:default handler(3) for breakpoint exception
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPHANDLER void handle_breakpoint(struct ctx_reg *reg)
{
	int err;
	
	vd_printf("breakpoint");
	
	vd_printf("reg->eip:0x%08X\n", reg->eip);
	
	reg->eip--;
	
	err = restore_bp((void*)reg->eip);
	
	if (!err) {
		vd_printf("resotre\n");
		ASM (
		"jmp	flush_after_restore	\n\t"
		"flush_after_restore: \n"
		);
	} else {
		vd_printf("\n");
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:insert_bp
 Input		:void *addr
 		 < address to insert a break point >
 Output		:void
 Return		:void
 Description	:insert a brea point
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void insert_bp(void *addr)
{
	unsigned char *insertee = (unsigned char*)addr;
	
	debug_bp.i = *insertee;
	debug_bp.address = (unsigned long)addr;
	
	printf("instruction:0x%02X\n", debug_bp.i);
	
	*insertee = DEBUG_TRAP_INST;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:restore_bp
 Input		:void *addr
 		 < address to restore original instruction >
 Output		:void
 Return		:int
 		 < result >
 Description	:restore from break debug
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int restore_bp(void *addr)
{

	if (debug_bp.address && (debug_bp.address == (unsigned long)addr)) {
		unsigned char *restore;
		
		restore = (unsigned char*)debug_bp.address;
		
		*restore = debug_bp.i;
		
		debug_bp.address = 0;
		
		return(0);
	}
	
	return(1);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:insert_inf_loop
 Input		:void *addr
 		 < address to insert a infinite loop >
 Output		:void
 Return		:void
 Description	:insert a infinite loop
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void insert_inf_loop(void *addr)
{
	unsigned char *insertee = (unsigned char*)addr;
	
	debug_bp.i = *insertee;
	debug_bp.address = (unsigned long)addr;
	
	*insertee = 0xEB;
	*(insertee + 1) = 0xFE;
}

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
