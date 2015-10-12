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
#	Makefile
#		for GNU make
#
#	device driver : console/serial (em1d)

MACHINE = x86
TETYPE = std

SRC_SYSDEP = ns16450.c

# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

DRIVER_INSTALLDIR = $(BD)/driver/$(TETYPE)_$(MACHINE)

HEADER += $(BD)/include/tkse $(BD)/tkse/include

# ----------------------------------------------------------------------------

TARGET = console

S = ../src

VPATH = $(S)
HEADER += $(S)

SRC = main.c consmlock.c console_drv.c sharemgr.c line_drv.c
SRC += $(SRC_SYSDEP)

OBJ = $(addsuffix .o, $(basename $(SRC)))

CFLAGS += $(CFLAGS_WARNING)

# ----------------------------------------------------------------------------

.PHONY: all clean install source clean_source clean_all clean_install  $(DRIVER_INSTALLDIR)/%

ALL = $(TARGET).o

all:	$(ALL)
	$(MAKE) -f mksvc

$(TARGET).o: $(OBJ)
	$(LINK_R.o) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(OBJ) $(ALL) $(DEPS)
	$(MAKE) -f mksvc clean

source:
#	$(MAKE) -f mksvc source

clean_source:
#	$(MAKE) -f mksvc clean_source

clean_all:
	$(RM) $(OBJ) $(ALL) $(DEPS)
	$(MAKE) -f mksvc clean_all

install: $(addprefix $(DRIVER_INSTALLDIR)/, $(ALL))
	$(MAKE) -f mksvc install

clean_insatll:
	echo "clean_install"


$(DRIVER_INSTALLDIR)/%: %
#	$(BD)/etc/backup_copy -t  $< $(DRIVER_INSTALLDIR)
	$(CP)  $< $(DRIVER_INSTALLDIR)

ifdef DEPENDENCIES_OUTPUT
  $(DEPS): ; touch $(DEPS)
else
  $(DEPS): $(SRC) ; $(MAKEDEPS) $@ $?
endif
-include $(DEPS)
