include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = include
HAVE_DYNAMIC = yes

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
SYNTAX_SRC += $(wildcard syntax/*.c)
SYNTAX = $(SYNTAX_SRC:syntax/%.c=$(BUILDDIR)/%.so)

CFLAGS += -I$(INCLUDEDIR)
CFLAGS += -DTVI_VERSION='"$(VERSION)"' -DPREFIX='"$(PREFIX)"'
CFLAGS += --std=c99 -D_POSIX_C_SOURCE=200809L

ALL = $(BUILDDIR)/tvi
ifeq ($(HAVE_DYNAMIC),yes)
	ALL += $(SYNTAX)
	CFLAGS += $(RDYNAMIC)
endif

all : $(ALL)

$(BUILDDIR)/tvi : $(OBJ)
	@echo '[linking into $@]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ $^ $(CFLAGS)

$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)

$(BUILDDIR)/%.so : syntax/%.c
	@echo '[compiling $^]'
	@$(CC) -o $@ -shared -fPIC $^ $(CFLAGS)

install-syntax : all
	@echo '[installing syntax libs]'
	@mkdir -p $(DESTDIR)$(PREFIX)/lib/tvi
	@cp $(SYNTAX) $(DESTDIR)$(PREFIX)/lib/tvi
	@echo '[install syntax aliases]'
	@rm -f $(DESTDIR)$(PREFIX)/lib/tvi/h.so && ln -s c.so $(DESTDIR)$(PREFIX)/lib/tvi/h.so

INSTALL =
ifeq ($(HAVE_DYNAMIC),yes)
	INSTALL = install-syntax
endif

install : all $(INSTALL)
	@echo '[installing tvi]'
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp $(BUILDDIR)/tvi $(DESTDIR)$(PREFIX)/bin/tvi
	@echo '[installing manual]'
	@mkdir -p $(DESTDIR)$(PREFIX)/share/tvi
	@cp help.txt $(DESTDIR)$(PREFIX)/share/tvi

uninstall :
	rm -fr $(DESTDIR)$(PREFIX)/bin/tvi $(DESTDIR)$(PREFIX)/share/tvi $(DESTDIR)$(PREFIX)/lib/tvi

clean :
	rm -fr build

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/tvi install uninstall clean
