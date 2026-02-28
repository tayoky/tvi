include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = include

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += -I$(INCLUDEDIR)
CFLAGS += -DTASH_VERSION='"$(VERSION)"'
CFLAGS += --std=c99 -D_POSIX_C_SOURCE=200809L

all : $(BUILDDIR)/tvi

test : $(BUILDDIR)/tvi
	@$(CC) -o $@ $^

$(BUILDDIR)/tvi : $(OBJ)
	@echo '[linking into $@]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ $^ $(CFLAGS) ../tlibc/build/libc/stdlib/qsort.o


$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)

install : all
	@echo '[installing tvi]'
	@mkdir -p $(PREFIX)/bin
	@cp $(BUILDDIR)/tvi $(PREFIX)/bin/tvi

uninstall :
	rm -f $(PREFIX)/bin/tvi

clean :
	rm -fr build

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/tvi install uninstall clean
