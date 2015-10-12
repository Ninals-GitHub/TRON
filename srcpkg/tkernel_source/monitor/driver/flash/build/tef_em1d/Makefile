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

#	T-Monitor : flash (em1d)

MACHINE = em1d
TETYPE = tef

SRC_SYSDEP    = setup-em1d.c
SRC_WR_SYSDEP = reset-em1d.c cfi_16x1.c cfi.c

# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

TMONITOR_INSTALLDIR = $(BD)/monitor/bin/$(TETYPE)_$(MACHINE)

HEADER = $(BD)/include/tkse $(BD)/tkse/include $(BD)/include
HEADER += $(BD)/monitor/include

# ----------------------------------------------------------------------------

TARGET = flash

S = ../../src

VPATH = $(S)
HEADER += $(S)

SRC     = main.c
SRC    += $(SRC_SYSDEP)
SRC_WR  =
SRC_WR += $(SRC_WR_SYSDEP)

OBJ    = $(addsuffix .o, $(basename $(SRC)))
OBJ_WR = $(addsuffix .o, $(basename $(SRC_WR)))

CFLAGS += $(CFLAGS_WARNING)

# adjust for flashwr object
REN_SECTION =	--rename-section .text=flashwr.text \
		--rename-section .rodata=flashwr.rodata
KEEP_SYMBOL =	--keep-global-symbol=flashwr \
		--keep-global-symbol=FROM_SECSZ

# ----------------------------------------------------------------------------

.PHONY: all clean install

ALL = $(TARGET).o

all: $(ALL)

$(TARGET).o: $(OBJ) flashwr.o
	$(LINK_R.o) $^ $(OUTPUT_OPTION)

flashwr.o: $(OBJ_WR)
	$(LINK_R.o) $^ -lgcc $(OUTPUT_OPTION)
	$(OBJCOPY) $(REN_SECTION) $(KEEP_SYMBOL) $@

clean:
	$(RM) $(OBJ) $(OBJ_WR) flashwr.o $(ALL) $(DEPS)

install: $(addprefix $(TMONITOR_INSTALLDIR)/, $(ALL))

$(TMONITOR_INSTALLDIR)/%: %
#	$(BD)/etc/backup_copy -t -d !OLD $< $(TMONITOR_INSTALLDIR)

ifdef DEPENDENCIES_OUTPUT
  $(DEPS): ; touch $(DEPS)
else
  $(DEPS): $(SRC) ; $(MAKEDEPS) $@ $?
endif
-include $(DEPS)
