#
# ----------------------------------------------------------------------
#    T-Kernel 2.0 Software Package
#
#    Copyright 2011 by Ken Sakamura.
#    This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#    Modified by T-Engine Forum at 2012/11/07.
#    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
#
# ----------------------------------------------------------------------
#

#
#	Makefile for gmake
#	sysinit (system-dependent description)
#

# T-Engine type
MACHINE = em1d
TETYPE = tef

# default rules
include ../../../../etc/makerules

# source files (system-dependent)
SRC = devinit.c cntwus.c

# Extension source files
VPATH += ../../../extension/startup/noext
SRC += subsystem.c

# include main makefile (common description)
include ../../src_t2ex/Makefile.common
