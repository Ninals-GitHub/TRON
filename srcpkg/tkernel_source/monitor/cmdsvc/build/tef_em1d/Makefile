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
#     Modified by Nina Petipa at 2015/09/22
#
# ----------------------------------------------------------------------
#

#	T-Monitor : cmdsvc (em1d)

MACHINE = em1d
TETYPE = tef

SRC_SYSDEP = monhdr.S monent.c chkaddr.c register.c break.c \
	disassemble.c misc.c step.c

# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

TMONITOR_INSTALLDIR = $(BD)/monitor/bin/$(TETYPE)_$(MACHINE)

HEADER = $(BD)/include/tkse $(BD)/tkse/include $(BD)/include
HEADER += $(BD)/monitor/include

# ----------------------------------------------------------------------------

TARGET = cmdsvc
TARGET2 = wrkbuf

S = ../../src

VPATH = $(S):$(S)/armv6
HEADER += $(S)

SRC = command.c helpmsg.c svc.c memory.c \
	console.c load.c boot.c
SRC += $(SRC_SYSDEP)

OBJ = $(addsuffix .o, $(basename $(SRC)))

CFLAGS += $(CFLAGS_WARNING)

# ----------------------------------------------------------------------------

.PHONY: all clean install

ALL = $(TARGET).o $(TARGET2).o

all: $(ALL)

$(TARGET).o: $(OBJ)
	$(LINK_R.o) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(OBJ) $(ALL) $(DEPS)

install: $(addprefix $(TMONITOR_INSTALLDIR)/, $(ALL))

$(TMONITOR_INSTALLDIR)/%: %
#	$(BD)/etc/backup_copy -t -d !OLD $< $(TMONITOR_INSTALLDIR)

ifdef DEPENDENCIES_OUTPUT
  $(DEPS): ; touch $(DEPS)
else
  $(DEPS): $(SRC) ; $(MAKEDEPS) $@ $?
endif
-include $(DEPS)
