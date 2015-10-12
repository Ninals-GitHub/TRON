/*
 *	Extended SVC parameter packet
 *
 */

#include <basic.h>
#include <device/share.h>
#include <sys/str_align.h>
#include "fnshare.h"

typedef struct {
	INT dintno;	_align64
	FP inthdr;	_align64
} CONSIO_DEF_INTHDR_PARA;

