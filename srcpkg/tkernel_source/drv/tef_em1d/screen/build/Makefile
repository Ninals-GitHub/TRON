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

#
#	Makefile
#		for GNU make
#
#	device driver : screen (em1d)

MACHINE = em1d
TETYPE = tef

SRC_SYSDEP = em1d512.c

# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

DRIVER_INSTALLDIR = $(BD)/driver/$(TETYPE)_$(MACHINE)

HEADER += $(BD)/include/tkse $(BD)/tkse/include

# ----------------------------------------------------------------------------

TARGET = screen

S = ../src

VPATH = $(S)
HEADER += $(S)

SRC = main.c common.c conf.c
SRC += $(SRC_SYSDEP)

OBJ = $(addsuffix .o, $(basename $(SRC)))

CFLAGS += $(CFLAGS_WARNING)

# ----------------------------------------------------------------------------

.PHONY: all clean install

ALL = $(TARGET).o

all: $(ALL)

$(TARGET).o: $(OBJ)
	$(LINK_R.o) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(OBJ) $(ALL) $(DEPS)

install: $(addprefix $(DRIVER_INSTALLDIR)/, $(ALL))

$(DRIVER_INSTALLDIR)/%: %
#	$(BD)/etc/backup_copy -t -d !OLD $< $(DRIVER_INSTALLDIR)

ifdef DEPENDENCIES_OUTPUT
  $(DEPS): ; touch $(DEPS)
else
  $(DEPS): $(SRC) ; $(MAKEDEPS) $@ $?
endif
-include $(DEPS)
