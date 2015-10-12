#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the T-License 2.0.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by T-Engine Forum at 2011/09/08.
#     Modified by T-Engine Forum at 2012/11/07.
#
# ----------------------------------------------------------------------
#

#	T-Monitor : tmmain (em1d)

MACHINE = em1d
TETYPE = tef

SRC_SYSDEP =

# ----------------------------------------------------------------------------

DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

include $(BD)/etc/makerules

TMONITOR_INSTALLDIR = $(BD)/monitor/bin/$(TETYPE)_$(MACHINE)

HEADER = $(BD)/include/tkse $(BD)/tkse/include $(BD)/include
HEADER += $(BD)/monitor/include

# ----------------------------------------------------------------------------

# version
VER := 2.01.00

# T-Monitor boot message
define VERSION
char const Version[] = "$(VER)";
char const *Title[] = {
  "T-Monitor/$(TETYPE)_$(MACHINE) Version $(VER)",
  0,
  "@(#)$(shell LC_TIME=C date)"
};
endef

# ----------------------------------------------------------------------------

TARGET = tmonitor

S = ../../src

VPATH = $(S)
HEADER += $(S)

SRC = float_dummy.c
SRC += $(SRC_SYSDEP)

OBJ	= $(addsuffix .o, $(basename $(SRC)))

CFLAGS += $(CFLAGS_WARNING)

# ----------------------------------------------------------------------------

M_OBJ =	$(TMONITOR_INSTALLDIR)/hwdepend.o \
	$(TMONITOR_INSTALLDIR)/cmdsvc.o \
	$(TMONITOR_INSTALLDIR)/wrkbuf.o \
	$(TMONITOR_INSTALLDIR)/sio.o \
	$(TMONITOR_INSTALLDIR)/flash.o \
	$(TMONITOR_INSTALLDIR)/memdisk.o

LDLIBS = -lstr -lsvc -lgcc

LDFLAGS += -static -nostdlib -T $(LINKSCRIPT)

ifneq ($(filter ram, $(options)), )
  # RAM version, for debug purpose
  LINKSCRIPT = monitor-ram.lnk
else
  LINKSCRIPT = monitor.lnk
endif

# ----------------------------------------------------------------------------

.PHONY: all clean install

ALL = $(TARGET).mot $(TARGET).map

all: $(ALL)

$(TARGET).mot:	$(TARGET)
	$(OBJCOPY) -O srec --srec-forceS3 --srec-len 32 $< $@

$(TARGET): $(M_OBJ) $(OBJ) version.o
	$(CC) $(TARGET_ARCH) $(LDFLAGS) $^ $(LDLIBS) $(OUTPUT_OPTION)

version.c: Makefile $(OBJ) $(M_OBJ)
	echo "$$VERSION" > version.c

clean:
	$(RM) version.[co] $(OBJ) $(SRC.C) $(TARGET) $(ALL) $(DEPS)

install: $(addprefix $(EXE_INSTALLDIR)/, $(ALL))

ifdef DEPENDENCIES_OUTPUT
  $(DEPS): ; touch $(DEPS)
else
  $(DEPS): $(SRC) ; $(MAKEDEPS) $@ $?
endif
-include $(DEPS)

$(SRC.C):
