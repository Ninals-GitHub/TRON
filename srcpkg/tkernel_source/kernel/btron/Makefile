#
# ----------------------------------------------------------------------
#    T2EX Software Package
#
#    Copyright 2012 by Ken Sakamura.
#    This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
#    Modified by Nina Petipa at 2015/10/28.
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
#	T-Kernel/OS (common description)
#

# T-Engine type
MACHINE = x86
TETYPE = std

# default rules
include ../../etc/makerules

# source files (system-dependent)
SRC =

# source file dependencies (generated automatically)
DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

# ----------------------------------------------------------------------------

# target object
TARGET	= kernel.o

# btron source file path
S	= .

# common source files
SRC	+=	bprocess.c fork.c exec.c

VPATH	+= $(S)
HEADER	+= $(S)

# additional source files (cpu-dependent)
VPATH   += ../sysdepend_t2ex/cpu/$(MACHINE) ../sysdepend/cpu/$(MACHINE)
HEADER	+= ../sysdepend_t2ex/cpu/$(MACHINE) ../sysdepend/cpu/$(MACHINE)

# additional source files (device-dependent)
VPATH   += ../sysdepend_t2ex/device/$(TETYPE)_$(MACHINE) ../sysdepend/device/$(TETYPE)_$(MACHINE)
HEADER	+= ../sysdepend_t2ex/device/$(TETYPE)_$(MACHINE) ../sysdepend/device/$(TETYPE)_$(MACHINE)

OBJ = $(addsuffix .o, $(basename $(SRC)))

ASM_SRC = $(filter %.S, $(SRC))
ASM_OBJ	= $(addsuffix .o, $(basename $(ASM_SRC)))

# compiler options
CFLAGS += $(CFLAGS_WARNING)

CPPFLAGS += -DTKERNEL_CHECK_CONST

# ----------------------------------------------------------------------------

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LINK_R.o) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(OBJ) $(TARGET) $(DEPS)

# generate dependencies
$(DEPS): ; touch $(DEPS)
-include $(DEPS)
