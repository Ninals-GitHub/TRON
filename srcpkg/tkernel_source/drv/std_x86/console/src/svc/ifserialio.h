/*
 *	Extended SVC parameter packet
 *
 */

#include <basic.h>
#include <device/serialio.h>
#include <sys/str_align.h>
#include "fnserialio.h"

typedef struct {
	W port;	_align64
	B* buf;	_align64
	W len;	_align64
	W *alen;	_align64
	W tmout;	_align64
} SERIAL_SERIAL_IN_PARA;

typedef struct {
	W port;	_align64
	B* buf;	_align64
	W len;	_align64
	W *alen;	_align64
	W tmout;	_align64
} SERIAL_SERIAL_OUT_PARA;

typedef struct {
	W port;	_align64
	W kind;	_align64
	UW *arg;	_align64
} SERIAL_SERIAL_CTL_PARA;

