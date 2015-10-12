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
 *	@(#)pstartup.c (crttk)
 *
 *	Manager startup
 */

#include <basic.h>
#include <tk/typedef.h>
#include <tk/errno.h>

/*
 * Compiler-dependent initialization/finalization sequence
 */
IMPORT void _init_compiler( void );
IMPORT void _fini_compiler( void );

/*
 * Library (libtk) initialization/termination sequence
 */
IMPORT	void	_InitLibtk( void );
IMPORT	void	_FinishLibtk( void );

/*
 * Manager entry
 */
IMPORT ER main( INT ac, UB *av[] );

/*
 * Manager startup
 */
EXPORT ER _P_startup( INT ac, UB *av[] )
{
	ER	ercd;

	if ( ac >= 0 ) {
		/* Library initialization */
		_InitLibtk();

		/* Compiler-dependent initialization/finalization sequence */
		_init_compiler();
	}

	/* main() execute */
	ercd = main(ac, av);

	/* Library function termination sequence is performed in the
	 * event of startup failure (main returns an error during startup)
	 * and successful termination (main does not return an error
	 * at termination).
	 */
	if ( (ac >= 0 && ercd <  E_OK) || (ac <  0 && ercd >= E_OK) ) {
		/* Compiler-dependent finalization sequence */
		_fini_compiler();

		/* Library termination sequence */
		_FinishLibtk();
	}

	return ercd;
}
