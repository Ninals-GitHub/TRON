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
 *	help.h
 *
 *       help message definitions
 */

/*
 * help message definitions
 */
typedef struct help	HELP;
struct help {
	void (*prfn)( const HELP* );	/* display function */
	const UB	*msg;		/* message */
};

/*
 * display help message
 */
IMPORT void printHelp( const HELP *help );

/*
 * help message data
 */
IMPORT	const HELP	helpALL;
IMPORT	const HELP	helpD, helpDB, helpDH, helpDW;
IMPORT	const HELP	helpM, helpMB, helpMH, helpMW;
IMPORT	const HELP	helpF, helpFB, helpFH, helpFW;
IMPORT	const HELP	helpSC, helpSCB, helpSCH, helpSCW;
IMPORT	const HELP	helpCMP, helpMOV;
IMPORT	const HELP	helpIB, helpIH, helpIW;
IMPORT	const HELP	helpOB, helpOH, helpOW;
IMPORT	const HELP	helpDA, helpR, helpB, helpBC;
IMPORT	const HELP	helpG, helpS, helpN, helpBTR, helpLO;
IMPORT	const HELP	helpRD, helpWD, helpID, helpBD;
IMPORT	const HELP	helpKILL, helpWROM, helpFLLO;
IMPORT	const HELP	helpH, helpEX;
