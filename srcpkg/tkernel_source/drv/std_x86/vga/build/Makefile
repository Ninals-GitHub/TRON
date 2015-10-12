#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2015 by by Nina Petipa.
#     This software is distributed under the T-License 2.0.
# ----------------------------------------------------------------------
#
# ----------------------------------------------------------------------
#

#
#	Makefile
#		for GNU make
#
#	device driver : vga (x86)

MACHINE = x86
TETYPE = std

SRC_SYSDEP =

# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

DRIVER_INSTALLDIR = $(BD)/driver/$(TETYPE)_$(MACHINE)

HEADER = $(BD)/include/tkse $(BD)/tkse/include $(BD)/include

# ----------------------------------------------------------------------------

TARGET = vga

S = ../src

VPATH = $(S)
HEADER += $(S)

SRC = textmode.c

OBJ = $(addsuffix .o, $(basename $(SRC)))

CFLAGS += $(CFLAGS_WARNING)

# ----------------------------------------------------------------------------

.PHONY: all clean install $(DRIVER_INSTALLDIR)/%

ALL = $(TARGET).o

all: $(ALL)

$(TARGET).o: $(OBJ)
	$(LINK_R.o) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(OBJ) $(ALL) $(DEPS)

install: $(addprefix $(DRIVER_INSTALLDIR)/, $(ALL))

clean_install:

$(DRIVER_INSTALLDIR)/%: %
#	$(BD)/etc/backup_copy -t -d !OLD $< $(DRIVER_INSTALLDIR)
	$(CP) $< $(DRIVER_INSTALLDIR)

ifdef DEPENDENCIES_OUTPUT
  $(DEPS): ; touch $(DEPS)
else
  $(DEPS): $(SRC) ; $(MAKEDEPS) $@ $?
endif
-include $(DEPS)
