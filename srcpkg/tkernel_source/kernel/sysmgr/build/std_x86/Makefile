#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the T-License 2.0.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by T-Engine Forum at 2012/11/07.
#     Modified by Nina Petipa at 2015/07/28
#
# ----------------------------------------------------------------------
#

#
#	Makefile for gmake
#	T-Kernel/System Manager (system-dependent description)
#

# T-Engine type
MACHINE = x86
TETYPE = std

# default rules
include ../../../../etc/makerules

# source files (system-dependent)
#SRC = chkplv.c power.c cache.c bitop.c
SRC = chkplv.c power.c cache.c

# Extension source files
VPATH += ../../../extension/memory/nommu
SRC += memmgr.c segmgr.c

# include main makefile (common description)
include ../../src/Makefile.common
