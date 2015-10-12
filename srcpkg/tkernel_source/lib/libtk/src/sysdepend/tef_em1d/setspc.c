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
 *	@(#)setspc.c (libtk/EM1-D512)
 *
 *	Address space control
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/sysdef.h>
#include "getsvcenv.h"

/*
 * Set task address space
 */
EXPORT ER SetTaskSpace( ID taskid )
{
	UW	taskmode;
	ER	ercd;

	if ( taskid == TSK_SELF ) {
		/* Set current CPL in PPL */
		taskmode = getsvcenv();
		SCInfo.taskmode ^= (taskmode ^ (taskmode << 16)) & TMF_PPL(3);
	} else {
		T_EIT		r_eit;
		T_CREGS		r_cregs;
		T_TSKSPC	tskspc;

		/* Get logical space/taskmode for taskid tasks */
		ercd = tk_get_reg(taskid, NULL, &r_eit, &r_cregs);
		if ( ercd < E_OK ) {
			goto err_ret;
		}

		/* Change to logical space for nominated tasks */
		tskspc.uatb = r_cregs.uatb;
		tskspc.lsid = r_cregs.lsid;
		ercd = tk_set_tsp(TSK_SELF, &tskspc);
		if ( ercd < E_OK ) {
			goto err_ret;
		}

		/* Change to PPL for nominated tasks */
		taskmode = getsvcenv();
		SCInfo.taskmode ^= (taskmode ^ r_eit.taskmode) & TMF_PPL(3);
	}

	return E_OK;

err_ret:
	return ercd;
}
