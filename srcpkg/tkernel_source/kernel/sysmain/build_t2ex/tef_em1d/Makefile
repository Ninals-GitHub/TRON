#
# ----------------------------------------------------------------------
#    T2EX Software Package
#
#    Copyright 2012 by Ken Sakamura.
#    This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
#    Modified by T-Engine Forum at 2014/07/31.
#    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
#
# ----------------------------------------------------------------------
#

#
# This software package is available for use, modification, 
# and redistribution in accordance with the terms of the attached 
# T-License 2.x.
# If you want to redistribute the source code, you need to attach 
# the T-License 2.x document.
# There's no obligation to publish the content, and no obligation 
# to disclose it to the TRON Forum if you have modified the 
# software package.
# You can also distribute the modified source code. In this case, 
# please register the modification to T-Kernel traceability service.
# People can know the history of modifications by the service, 
# and can be sure that the version you have inherited some 
# modification of a particular version or not.
#
#    http://trace.tron.org/tk/?lang=en
#    http://trace.tron.org/tk/?lang=ja
#
# As per the provisions of the T-License 2.x, TRON Forum ensures that 
# the portion of the software that is copyrighted by Ken Sakamura or 
# the TRON Forum does not infringe the copyrights of a third party.
# However, it does not make any warranty other than this.
# DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
# responsible for any consequences or damages caused directly or
# indirectly by the use of this software package.
#
# The source codes in bsd_source.tar.gz in this software package are 
# derived from NetBSD or OpenBSD and not covered under T-License 2.x.
# They need to be changed or redistributed according to the 
# representation of each source header.
#

#
#	Makefile for gmake
#	sysmain (system-dependent description)
#

# T-Engine type
MACHINE = em1d
TETYPE	= tef

# ----------------------------------------------------------------------------
# T2EX definitions
T2EX = _t2ex

# use T2EX memory management
T2EX_MM = _t2ex

# use task fault function of T2EX memory management
T2EX_MM_USE_TASKEXCEPTION = 1

# use default memory fault handlers
T2EX_MM_USE_DEFAULT_FAULT_HANDLER = 1

# use T2EX file management
T2EX_FS = yes

# use T2EX network communication
T2EX_NET = yes

# use T2EX calendar
T2EX_DT = yes

# use T2EX program load
T2EX_PM = yes

# use T2EX network sample programs
T2EX_NET_SAMPLE = yes

# ----------------------------------------------------------------------------
# default rules
include $(BD)/etc/makerules

# ----------------------------------------------------------------------------
# source files
SRC_SYS	= usermain_t2ex.c

# source files for simple command interpreter
SRC_SYS	+= appl_main.c command.c

# user-level source files
SRC_USR	=

# user-level libraries
LDUSRLIBS =

# ----------------------------------------------------------------------------
# device driver definitions
DRV_CONSOLE = yes
DRV_CLOCK   = yes
DRV_SYSDISK = yes
DRV_SCREEN  = yes
DRV_KBPD    = yes
DRV_LOWKBPD = yes
DRV_NET     = yes

OBJ_DRIVER = $(BD)/driver/$(TETYPE)_$(MACHINE)/build
ifdef DRV_CONSOLE
  I_OBJ += $(OBJ_DRIVER)/console.o
  CFLAGS += -DDRV_CONSOLE
  LDUSRLIBS += -lconsolesvc
endif
ifdef DRV_CLOCK
  I_OBJ += $(OBJ_DRIVER)/clkdrv.o
  CFLAGS += -DDRV_CLOCK
  LDUSRLIBS += -lem1diic
endif
ifdef DRV_SYSDISK
  I_OBJ += $(OBJ_DRIVER)/sysdsk.o
  CFLAGS += -DDRV_SYSDISK
endif
ifdef	DRV_SCREEN
  I_OBJ += $(OBJ_DRIVER)/screen.o
  CFLAGS += -DDRV_SCREEN
endif
ifdef	DRV_KBPD
  I_OBJ += $(OBJ_DRIVER)/kbpd.o
  CFLAGS += -DDRV_KBPD=1
endif
ifdef	DRV_LOWKBPD
  I_OBJ += $(OBJ_DRIVER)/lowkbpd.o
  CFLAGS += -DDRV_LOWKBPD
endif
ifdef	DRV_NET
  I_OBJ += $(OBJ_DRIVER)/netdrv.o
  CFLAGS += -DDRV_NET
endif

# ----------------------------------------------------------------------------
# additional libraries
LDUSRLIBS += -ldrvif -lstr
ifdef	T2EX
  # additional header file path for t2ex
  HEADER := $(BD)/include/t2ex $(HEADER)
endif

# ----------------------------------------------------------------------------
# network sample programs
ifdef	T2EX_NET_SAMPLE
  # include network_sample programs
  include ../../src/network_sample/Makefile.sample
  CFLAGS += -DNET_SAMPLE
endif

# ----------------------------------------------------------------------------
# include main makefile (common description)
include ../Makefile.common

