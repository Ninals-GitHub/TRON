/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	disasemble.c
 *
 *       disassember
 */

#include "../cmdsvc.h"

LOCAL	UB	*make_hex(UB *str, UB byte)
{
	LOCAL	const UB	hex[] = "0123456789ABCDEF";

	*str++ = hex[(byte >> 4) & 0x0f];
	*str++ = hex[(byte >> 0) & 0x0f];

	return str;
}

/*
        disassembler main body

        * disassembly is not fully done. But, during step tracing,
          memory content is shown (this much is implemented).
          the content of *naddr is meangless
 */
EXPORT	ER	disAssemble(UW *saddr, UW *naddr, UB *str)
{
	W	len;
	UW	inst, addr;

	len = (*saddr & 0x1) ? 2 : 4;		// Thumb or Arm instruction
	addr = (*saddr &= ~(len - 1));		// address adjustment

        // extract op code
	if (readMem(addr, &inst, len, 2) != len) return E_MACV;

        // binary dump
	if (len == 4) {
		str = make_hex(str, inst >> 24);
		str = make_hex(str, inst >> 16);
	}
	str = make_hex(str, inst >> 8);
	str = make_hex(str, inst >> 0);

	*str = '\0';

	return E_OK;
}
