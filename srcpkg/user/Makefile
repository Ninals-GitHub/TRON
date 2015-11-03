#
# ----------------------------------------------------------------------
#    T2EX Software Package
#
#    Copyright 2015 by Nina Petipa.
#    This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#
# ----------------------------------------------------------------------
#

CC := $(GNUX86)/bin/i686-pc-linux-gnu-gcc
HEADER += $(BD)/include



CFLAGS += $(HEADER:%=-I%) $(_CPUTYPE_FLAGS) $(_CODESET_FLAGS) -static
LDFLGAS += aaa

# ----------------------------------------------------------------------------

# target object
TARGET	= test

# ----------------------------------------------------------------------------

.PHONY: all clean

ALL = $(TARGET)

all: $(ALL)

clean:
	$(RM) $(OBJ) $(ALL) $(DEPS)

