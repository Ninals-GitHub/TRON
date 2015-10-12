/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	objname.c (T-Kernel/DS)
 *	Object name support
 */

#include <tk/kernel.h>
#include <libstr.h>

#if USE_DBGSPT

#if USE_OBJECT_NAME
LOCAL ER object_getname( UINT objtype, ID objid, UB **name)
{
	ER	ercd;

	switch (objtype) {
#ifdef NUM_TSKID
	  case TN_TSK:
		{
			IMPORT ER task_getname(ID id, UB **name);
			ercd = task_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_SEMID
	  case TN_SEM:
		{
			IMPORT ER semaphore_getname(ID id, UB **name);
			ercd = semaphore_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_FLGID
	  case TN_FLG:
		{
			IMPORT ER eventflag_getname(ID id, UB **name);
			ercd = eventflag_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_MBXID
	  case TN_MBX:
		{
			IMPORT ER mailbox_getname(ID id, UB **name);
			ercd = mailbox_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_MBFID
	  case TN_MBF:
		{
			IMPORT ER messagebuffer_getname(ID id, UB **name);
			ercd = messagebuffer_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_PORID
	  case TN_POR:
		{
			IMPORT ER rendezvous_getname(ID id, UB **name);
			ercd = rendezvous_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_MTXID
	  case TN_MTX:
		{
			IMPORT ER mutex_getname(ID id, UB **name);
			ercd = mutex_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_MPLID
	  case TN_MPL:
		{
			IMPORT ER memorypool_getname(ID id, UB **name);
			ercd = memorypool_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_MPFID
	  case TN_MPF:
		{
			IMPORT ER fix_memorypool_getname(ID id, UB **name);
			ercd = fix_memorypool_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_CYCID
	  case TN_CYC:
		{
			IMPORT ER cyclichandler_getname(ID id, UB **name);
			ercd = cyclichandler_getname(objid, name);
			break;
		}
#endif

#ifdef NUM_ALMID
	  case TN_ALM:
		{
			IMPORT ER alarmhandler_getname(ID id, UB **name);
			ercd = alarmhandler_getname(objid, name);
			break;
		}
#endif

	  default:
		ercd = E_PAR;

	}

	return ercd;
}
#endif /* USE_OBJECT_NAME */


SYSCALL ER _td_ref_dsname( UINT type, ID id, UB *dsname )
{
#if USE_OBJECT_NAME
	UB	*name_cb;
	ER	ercd;

	ercd = object_getname(type, id, &name_cb);
	if (ercd == E_OK) {
		strncpy((char*)dsname, (char*)name_cb, OBJECT_NAME_LENGTH);
	}

	return ercd;
#else
	return E_NOSPT;
#endif /* USE_OBJECT_NAME */
}

SYSCALL ER _td_set_dsname( UINT type, ID id, CONST UB *dsname )
{
#if USE_OBJECT_NAME
	UB	*name_cb;
	ER	ercd;

	ercd = object_getname(type, id, &name_cb);
	if (ercd == E_OK) {
		strncpy((char*)name_cb, (char*)dsname, OBJECT_NAME_LENGTH);
	}

	return ercd;
#else
	return E_NOSPT;
#endif /* USE_OBJECT_NAME */
}

#endif /* USE_DBGSPT */
