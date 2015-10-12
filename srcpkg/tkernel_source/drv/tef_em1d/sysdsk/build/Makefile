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
#	device driver : sysdsk (em1d)

MACHINE = em1d
TETYPE = tef


# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

DRIVER_INSTALLDIR = $(BD)/driver/$(TETYPE)_$(MACHINE)

HEADER += $(BD)/include/tkse $(BD)/tkse/include

# ----------------------------------------------------------------------------

TARGET = sysdsk

S = ../src

VPATH = $(S)
HEADER += $(S)

SRC = main.c accept.c common.c inthdr.c pccard.c \
	ata.c atacmd.c ataio.c mem.c

SOBJ =  ../obj/mmc-common.o ../obj/mmc-em1d512.o
OBJ = $(addsuffix .o, $(basename $(SRC)))

CFLAGS += -DUSE_MEMDRV -DUSE_MMCDRV
CFLAGS += -DUSE_MMC_MULTI -DUSE_MMC_SD4BIT

CFLAGS += $(CFLAGS_WARNING)

# ----------------------------------------------------------------------------

.PHONY: all clean install

ALL = $(TARGET).o

all: $(ALL)

$(TARGET).o: $(OBJ)
	$(LINK_R.o) $^ $(SOBJ) $(OUTPUT_OPTION)

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
