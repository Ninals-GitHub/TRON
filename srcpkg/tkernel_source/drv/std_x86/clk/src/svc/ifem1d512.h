/*
 *	Extended SVC parameter packet
 *
 */

#include <basic.h>
#include <device/em1d512_iic.h>
#include <sys/str_align.h>
#include "fnem1d512.h"

typedef struct {
	W ch;	_align64
	UH *cmddat;	_align64
	W words;	_align64
} H8IO_EM1D512_IICXFER_PARA;

typedef struct {
	W cs;	_align64
	UB *xmit;	_align64
	UB *recv;	_align64
	W len;	_align64
} H8IO_EM1D512_SPIXFER_PARA;

