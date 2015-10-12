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
 *	cmdsvc.h
 *
 *       T-Monitor command / SVC common processing definitions
 */

#include <tmonitor.h>
#include <tm/tmonitor.h>
#include <libstr.h>

#if CPU_ARMv6
#  include "armv6/cpudep.h"
#endif

/* ======================================================================== */
/*
 *       Hardware-independent
 */

#define	L_LINE		256		/* number of characters per line */
IMPORT	UB	lineBuf[L_LINE];	/* line buffer */

#define	WRKBUF_SZ	1024		/* must be larger than or equal to 1024 */
IMPORT	UB	wrkBuf[WRKBUF_SZ];	/* work buffer for various operations */

#define	L_BPCMD		80		/* breakpooint command length */

IMPORT	W	errinfo;		/* error information */

/*
 * display boot message
 */
IMPORT void dispTitle( void );

/*
 * command execution
 *       cmd     command string
 *               if it is NULL, we are wainting for command input
 *       fin     = 0 : execute cmd and then wait for command input
 *               > 0 : execute cmd and return
 *               < 0 : execute cmd and return (ignore execution such as GO,STEP, etc.)
 *               if cmd = NULL, fin is ignored. (equivalent to fin = 0)
 */
IMPORT void procCommand( UB *cmd, W fin );

/*
 * loading from serial line
 *       proto   [P_XMODEM] | [P_SFORM]  (other protocols are ignored)
 *		 P_XMODEM	XMODEM
 *               not specified  no control sequence
 *               P_SFORM         S-Record format
 *               not specficied            memory image (binary data)
 *       addr    load address
 *               In the case of P_SFORM, the initial ooad address is set to addr.
 *               Loading is done with the above adjustment. If addr = 0, adjustment is not madem, but
 *               load it to data address.
 *       range   valid load range
 *               range[0]  start address of the valid load range
 *               range[1]  end address of the valid load range
 *               range[2]  load offset
 *               load offset is valid only when P_SFORM is used and addr = 0.
 *               Its value is the sum of the load address added with the range[2] offset value.
 *               This is where loading takes place.
 *               range[0] and range[1] will return the final starting and loading address after the loading.
 *
 *               if range = NULL, below is assumed.
 *		 range[0] = 0x00000000
 *		 range[1] = 0xffffffff
 *		 range[2] = 0
 *       return value error code
 */
IMPORT ER doLoading( W proto, UW addr, UW range[3] );

/* load option (LOAD commands, etc.) */
#define	P_XMODEM	0x20		/* XMODEM */
#define	P_TEXT		0x10		/* no protocol */
#define	P_SFORM		0x02		/* S-Format */
#define	P_MEMIMG	0x01		/* memory image */

/*
 * Flash ROM disk write
 *       blksz = 0 set up
 *          Prepare writing to ROM disk and return the maximum ROM size
 *          in sz (bytes).
 *           Return the address to which the data to be written to ROM disk should be loaded.
 *       blksz > 0 written
 *           blksz       ROM disk block size
 *           sz          number of written blocks
 *       return value error code
 */
IMPORT W writeRda( UW blksz, UW *sz );

/*
 * disk boot
 *       devnm   device name (possibly with the partition number)
 *               if it is NULL, the standard search order is used to look for a bootable device.
 *       return value error code
 */
IMPORT ER bootDisk( const UB *devnm );

/*
 * monitor service call
 *       fno     function code
 *       p1-p4 parameters
 *       return value     return value of the service call
 */
IMPORT W procSVC( W fno, W p1, W p2, W p3, W p4 );

/* ======================================================================== */
/*
 *       Hardware-dependent
 */

/*
 * disassember
 *       saddr   pass the address where the instruction to be disassembled is.
 *               returns the adjusted address that points at the start of the instruction.
 *       naddr  returns the address of the next instruction. (if it is NULL, value is not returned)
 *       str     the buffer to store the disassembled instruction string (must be long enough)
 *       return value error code
 */
IMPORT ER disAssemble( UW *saddr, UW *naddr, UB *str );

/*
 * examine and obtain the breakpoint attribute
 *       examine the breakpoint attribute string specified by `name', and if it is legal,
 *       return its attribute code as return value.
 *      In the case of illegal attribute string, return an error (E_BPATR).
 */
IMPORT W getBreakAtr( UB *name );

/*
 * set breakpoint
 *       addr    address where breakpoint is set
 *       atr     breakpoint attribute
 *       cmd     command that is to be executed at the breakpoint (valid only when cmdlen > 0)
 *       cmdlen  cmd length
 *       return value error code
 */
IMPORT ER setBreak( UW addr, W atr, UB *cmd, W cmdlen );

/*
 * release breakpoint
 *       addr    address where breakpoint is to be released
 *               if it is 0, then release all breakpoints
 *       return value error code
 */
IMPORT ER clearBreak( UW addr );

/*
 * list all breakpoints
 */
IMPORT void dspBreak( void );

/*
 * release breakpoint temporarily (monitor entry)
 *       return value      if the current PC is not a breakpoint, returns 0.
 *               if it is a breakpoint, returns non-zero value.
 */
IMPORT W resetBreak( UW vec );

/*
 * set breakpoints (monitor exit)
 */
IMPORT void setupBreak( void );

/*
 * forcibly stop trace
 */
IMPORT void stopTrace( void );

/*
 * trace or normal execution
 *       trace   0 : GO command
 *               1 : STEP command
 *               2 : NEXT command
 *       pc      execution start address
 *       par     in the case of GO
 *                 tempoary breakpoint address (if 0, then no temporary breakpoionts)
 *               in the case of STEP/NEXT
 *                 number of steps (0 is regarded as 1)
 *       return value error code
 */
IMPORT ER goTrace( W trace, UW pc, UW par );

/*
 * break processing
 *       bpflg   return the value returned by resetBreak() as is.
 *       cmd     return the command to be executed at the breakpoint.
 *       return value     0 : return monitor immediately and contineu executing the user program.
 *                        1 : enter command processing of the monitor and execute cmd.
 */
IMPORT W procBreak( W bpflg, UB **cmd );

/*
 * search register
 *       Return the register number (0 and upward) for the register name `name'.
 *       grp     0 : exclude group names from the search.
 *               1 : include the group names in the search/
 *       name is L_REGNM bytes long, and if the result is not long enough, the remaining part is filled with space.
 *       L_REGNM is machine-dependent, but should be less than or equal to L_SYMBOL.
 *       if the name is invalid (not found), return -1.
 */
IMPORT W searchRegister( UB *name, W grp );

/*
 * obtain / set register value
 *       regno   register number
 *       val     value to set
 *       return value obtained set value(if there is an error, 0).
 *               *      set :  error code
 */
IMPORT UW getRegister( W regno );
IMPORT ER setRegister( W regno, UW val );

/*
 * display register value
 *       display the register or register group specified by `regno'.
 *       if regno < 0, thendisplay the default register group.
 */
IMPORT void dispRegister( W regno );

/*
 * obtain / set the current PC register
 */
IMPORT UW   getCurPC( void );
IMPORT void setCurPC( UW val );

/*
 * prepare to execute the boot program
 *       pass boot info to a boot program and start it from the address, `start'.
 *
 *      It means that we don't execute ROM kernel immediately, but we prepare so that upon return from the ordinary monitor,
 *       it gets executed.
 */
IMPORT void setUpBoot( void *start, BootInfo *bootinfo );

/*
 * Prepare ROM kernel execution
 *      It means that we don't execute ROM kernel immediately, but we prepare so that upon return from the ordinary monitor,
 *       it gets executed.
 */
IMPORT ER bootROM( void );

/*
 * Initialize address check data (executed upon monitor entry)
 */
IMPORT void initChkAddr( void );

/*
 * validate memory address
 *      check whether access to area starting from logical address, addr, and has the length of len bytes,
 *       return the corresponding physical address of addr in pa.
 *       rw = 0 for read / rw = 1 for write is checked for the access right check.
 *       Returns the length of accessible bytes for a consecutive region that starts from addr.
 *       if addr is inaccessible, then the size for the accesible region is 0.
 *       So, 0 is returned. In this case, error code is set to a global variable errinfo.
 *       the address returned to pa is machine-depdenent, and may not be a physical address always.
 */
IMPORT W chkMemAddr( UW addr, UW *pa, W len, W rw );

/*
 * validate I/O address
 *      heck whether we can access a region from an I/O address (logical address in the case of memory mapped I/O), with the len bytes length,
 *      the I/O address (physical address if memory mapped I/O is used) is returned
 *       in pa.
 *       Returns the length of accessible bytes for a consecutive region that starts from addr.
 *       if addr is inaccessible, then the size for the accesible region is 0.
 *       So, 0 is returned.
 * * the address returned to pa is machine-depdenent, and, in the case of memory mapped I/O.
 *         may not be a physical address always. if memory mapped I/O is not used,
 *          generally speaking, pa will return addr unmodiifed.
 */
IMPORT W chkIOAddr( UW addr, UW *pa, W len );

/*
 * Validate PC address
 *      If addr is valid then return 0, otherwise return -1.
 */
IMPORT W invalidPC( UW addr );

/*
 * Check whehter kill command can be executed
 *       If it can be executed, return 0, and if not, return -1.
 */
IMPORT W isKillValid( void );

/*
 * calling an external program
 */
IMPORT W callExtProg( FP entry );

/* ------------------------------------------------------------------------ */
