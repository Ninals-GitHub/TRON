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
 *	tmonitor.h
 *
 *       T-Monitor internal common definitions
 */

#ifndef __MONITOR_TMONITOR_H__
#define	__MONITOR_TMONITOR_H__

#include <basic.h>
#include <tk/errno.h>
#include <libstr.h>
#include "device.h"

/* CPU-dependent definitions */
#if CPU_ARM
#  include "arm/cpudepend.h"
#endif

/*
 * console display
 */
#define	DSP_S(s)	putString(s)
#define	DSP_LF		putChar('\n')
#define	DSP_SP		putChar(' ')
#define	DSP_CH(c)	putChar(c)
#define	DSP_02X(x)	putHex2(x)
#define	DSP_04X(x)	putHex4(x)
#define	DSP_08X(x)	putHex8(x)
#define	DSP_D(x)	putDec(x)
#define	DSP_F1(f0, a0)	\
	{DSP_##f0(a0);}
#define	DSP_F2(f0, a0, f1, a1)	\
	{DSP_##f0(a0); DSP_##f1(a1);}
#define	DSP_F3(f0, a0, f1, a1, f2, a2)	\
	{DSP_##f0(a0); DSP_##f1(a1); DSP_##f2(a2);}
#define	DSP_F4(f0, a0, f1, a1, f2, a2, f3, a3)	\
	{DSP_##f0(a0); DSP_##f1(a1); DSP_##f2(a2); DSP_##f3(a3);}
#define	DSP_F5(f0, a0, f1, a1, f2, a2, f3, a3, f4, a4)	\
	{DSP_##f0(a0); DSP_##f1(a1); DSP_##f2(a2); DSP_##f3(a3); DSP_##f4(a4);}

/*
 * error code
 *       common error codes with T-Kernel (the following is used)
 *	E_OK, E_MACV, E_PAR, E_LIMIT, E_NOSPT, E_NOEXS, E_IO, E_RONLY
 */
        /* original error code */
#define	E_END		(-1000)
#define	E_LESS		(-1001)
#define	E_CMD		(-1002)
#define	E_RANGE		(-1003)
#define	E_EMPTY		(-1004)
#define	E_ILREG		(-1005)
#define	E_PC		(-1006)
#define	E_BOOT		(-1007)
#define	E_ROM		(-1008)
#define	E_PROTECT	(-1009)
        /* LOAD command and friends */
#define	E_PROTO		(-1010)
#define	E_NOADDR	(-1011)
#define	E_LOADFMT	(-1012)
#define	E_LOAD		(-1013)
#define	E_CANCEL	(-1014)
#define	E_XMODEM	(-1015)
        /* BREAK command and friends */
#define	E_BPATR		(-1020)
#define	E_BPBAD		(-1021)
#define	E_BPDSLT	(-1022)
#define	E_BPROM		(-1023)
#define	E_BPCMD		(-1024)
#define	E_BPUDF		(-1025)
#define	E_HBPOVR	(-1026)
#define	E_SBPOVR	(-1027)

#define	E_ONOEXS	(-1030)

IMPORT char const Version[];	/* version number */
IMPORT char const *Title[];	 /* boot message */

/* ------------------------------------------------------------------------ */

/*
 * service call function code
 */
#define	TM_MONITOR	0
#define	TM_GETCHAR	1
#define	TM_PUTCHAR	2
#define	TM_GETLINE	3
#define	TM_PUTSTRING	4
#define	TM_COMMAND	5
#define	TM_READDISK	6
#define	TM_WRITEDISK	7
#define	TM_INFODISK	8
#define	TM_EXIT		9

#define	TM_EXTSVC	255

/* ======================================================================== */
/*
 *       hardware dependent processing (hwdepend)function
 */

/*
 * system basic set up
 *       boot    1 : boot is in progress
 *               0 : reset is in progress
 */
IMPORT void resetSystem( W boot );

/*
 * system termination
 *       reset    0 : power off
 *               -1 : reboot
 *               other: machine-dependent
 */
IMPORT void sysExit( W reset );

/*
 * processing at monitor entry and exit
 *       info and return value is defined in machine-dependent manner.
 */
IMPORT W enterMonitor( UW info );	/* entry */
IMPORT W leaveMonitor( UW info );	/* exit */

/*
 * EIT processing
 *       return value      0 : monitor should keep on running
 *                         1 : return from the interrupt handler
 */
IMPORT W procEIT( UW vec );

/*
 * Obtain boot selection information
 */
IMPORT W bootSelect( void );

/* bootSelect() return value */
#define	BS_MONITOR	0	/* boot monitor */
#define	BS_AUTO		1	/* automatic boot */

/*
 * boot device following the standard boot order
 *       return the device name that is the 'no'-th device in the standard boot order.
 *
 *       if no such device name exists (when 'no' is given as a value larger or equal to the last number), it is NULL.
 */
IMPORT const UB* bootDevice( W no );

IMPORT const UH BootSignature;	/* boot block signature */
IMPORT UB* const PBootAddr;	/* primary boot loader address */

/*
 * list of disk drives
 *       returns the disk drive device name, indicated by 'no' ( 0 - : a consecutive number )
 *       if no such device name exists (when 'no' is given as a value larger or equal to the last number), it is NULL.
 *       if attr is not NULL, disk driver attribute returns in `attr' )
 */
IMPORT const UB* diskList( W no, UW *attr );

/*
 * obtain switch status
 */
IMPORT UW getDipSw( void );

/*
 * set LED
 *       lower 16 bits of val value (1:ON 0:OFF)
 *       upper 16 bits of val mask (1: keep, 0: change)
 */
IMPORT void cpuLED( UW val );

/*
 * micro wait for a small amount of time
 *       wait time is not that accurate.
 */
IMPORT void waitMsec( UW msec );	/* milliseconds */
IMPORT void waitUsec( UW usec );	/* microseconds */
IMPORT void waitNsec( UW nsec );	/* nanoseconds */

/*
 *       cache control
 *       acts on the whole address space.
 */
IMPORT void FlushCache( void );		/* writeback and invalidate */
IMPORT void EnableCache( void );	/* enable cache */
IMPORT void DisableCache( void );	/* disable cache */

/* ------------------------------------------------------------------------ */

/*
 *       memory region definition
 *       the location of end is NOT included in the region. ((end - top) is the region size)
 *       end = 0x00000000, by the way, means 0x100000000.
 */
typedef struct {
	UW	top;		/* area start address */
	UW	end;		/* area end address */
	UW	attr;		/* attribute */
#if (CPU_ARM|CPU_I386) && VIRTUAL_ADDRESS
	UW	pa;		/* physical address | page attribute */
#endif
} MEMSEG;

/*
 * attribute attr
 *       if MSA_WRK is defined, make it so that it is found before MSA_OS.
 */
#define	MSA_ROM		0x0001	/* ROM       (read-only) */
#define	MSA_FROM	0x0002	/* Flash ROM (write_enabled) */
#define	MSA_RAM		0x0004	/* RAM */
#define	MSA_ERAM	0x0008	/* extended RAM */
#define	MSA_IO		0x0010	/* I/O */
#define	MSA_SRAM	0x0020	/* SRAM */
#define	MSA_HW		0x0fff	/* attribute related to hardware */

#define	MSA_MON		0x1000	/* monitor area        (area inside MSA_ROM/FROM ) */
#define	MSA_OS		0x2000	/* OS area        (area in MSA_RAM ) */
#define	MSA_WRK		0x4000	/* special work area (used by LH7A400) */

#define	MSA_RDA		0x10000	/* ROM disk area (area in MSA_FROM ) */
#define	MSA_RDB		0x20000	/* RAM disk area */

/* page attribute (ARM) 1st level page table */
#if CPU_ARM
#if CPU_ARMv6
#define	PGA_RW		0x00402	/* Kernel/RW (effective section, AP0='1') */
#define	PGA_RO		0x08402	/* Kernel/RO (effective section) AP0='1') */
#define	PGA_XN		0x00010	/* code execution prohibited */
#define	PGA_C		0x0100c	/* TEX0:C:B='111', Normal, WB/WA  */
#define	PGA_NC		0x01000	/* TEX0:C:B='100', Normal, CacheOff */
#define	PGA_D		0x00004	/* TEX0:C:B='001', Device, CacheOff */
#define	PGA_S		0x10000	/* shareable */
#endif	/*CPU_ARMv6*/
#endif	/*CPU_ARM*/

/*
 * obtaining memory region information
 *       no = 1 - (and up)
 *       'no'-th information in the region specified by the attr is returned.
 *       if attr = 0, no matter what the attribute is, 'no'-th information is returned unconditionally.
 *       if no such information is found, return NULL.
 */
IMPORT MEMSEG* MemArea( UW attr, W no );

/*
 * obtaining memory region information (specify address)
 *       within the region specified by `attr', return the information that surrounds the position specified by `addr'.
 *
 *       if no such information is found, return NULL.
 */
IMPORT MEMSEG* AddrMatchMemArea( UW addr, UW attr );

/*
 * Decide whether two memory regions are included in another.
 *      if the region, from `top' to `end', is completely included in the region specified by `attr',
 *      TRUE
 *       the location of end is NOT included in the region (end - top) is the region size
 *       end = 0x00000000, by the way, means 0x100000000.
 */
IMPORT BOOL inMemArea( UW top, UW end, UW attr );

/*
 * Decide whether two memory regions overlap with each other
 *       if the area, from top to end, is included even partially in the region specified by `attr' - 'end',
 *       it is TRUE
 *       the location of end is NOT included in the region. ((end - top) is the region size)
 *       end = 0x00000000, by the way, means 0x100000000.
 */
IMPORT BOOL isOverlapMemArea( UW top, UW end, UW attr );

/* ======================================================================== */
/*
 *       command / SVC processing (cmdsvc) function
 */

/*
 * console output
 *       XON/XOFF flow control
 *       check for CTRL-C input
 *       return value       0 : normal
 *                         -1 : CTRL-C input exists
 */
IMPORT W putChar( W c );
IMPORT W putString( const UB *str );

/*
 * console output (hexadecimal: 2, 4, or 8 columns)
 *       XON/XOFF flow control
 *       check for CTRL-C input
 *       return value       0 : normal
 *                         -1 : CTRL-C input exists
 */
IMPORT W putHex2( UB val );
IMPORT W putHex4( UH val );
IMPORT W putHex8( UW val );

/*
 * console output (decimal: 10 columns/zero-suppress supported)
 *       XON/XOFF flow control
 *       check for CTRL-C input
 *       return value       0 : normal
 *                         -1 : CTRL-C input exists
 */
IMPORT W putDec( UW val );

/*
 * console input (one character)
 *       if wait = TRUE, wait for input if FALSE, do not wait.
 *       return value       >= 0 : character
 *                            -1 : no input
 */
IMPORT W getChar( BOOL wait );

/*
 * console input (character string)
 *       line input with editing
 *       return value      >= 0 : number of input characters
 *                           -1 : CTRL-C was detected
 */
IMPORT W getString( UB *str );

/*
 * detect CTRL-C
 *       check if there is a history of control-C input to the console
 *       history is cleared
 *      return value      TRUE  : CTRL-C input exists
 *                        FALSE : CTRL-C input is absent
 */
IMPORT BOOL checkAbort( void );

/*
 *       memory & I/O access
 *       len     number of bytes
 *       unit    access unit (1=B 2=H 4=W)
 *       return value number of bytes really accessed
 *                    return 0 when there was an error or exception
 *
 *       writeMem() becomes fill operation if (unit | 0x10) is given. (only writeMem)
 *       in this case, the only leading data in buf is used.
 */
IMPORT W readMem( UW addr, void *buf, W len, W unit );
IMPORT W writeMem( UW addr, void *buf, W len, W unit );
IMPORT W readIO( UW addr, UW *data, W unit );
IMPORT W writeIO( UW addr, UW data, W unit );

/*
 * read character string
 *       read byte string up to maximum len bytes from addr to buf.
 *       return the length read (excluding the terminating '\0').
 *       if an error occurs (including the string longer than len bytes), return -1.
 */
IMPORT W readMemStr( UW addr, void *buf, W len );

/*
 * initialize breakpoint
 */
IMPORT void initBreak( void );

/*
 * Invoking user reset initialization routine
 */
IMPORT void callUserResetInit( void );

/* ======================================================================== */

#endif /* __MONITOR_TMONITOR_H__ */
