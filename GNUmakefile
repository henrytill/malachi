CFLAGS = -g -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
ALL_CFLAGS = -std=c11 -Iinclude $(CFLAGS)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

bindir = /bin
prefix = /usr/local
DESTDIR = $(prefix)

BINOUT = _bin

HEADERS =
HEADERS += include/config.h
HEADERS += include/error.h
HEADERS += include/path.h
HEADERS += include/platform.h

OBJECTS =
OBJECTS += src/config.o
OBJECTS += src/main.o
OBJECTS += src/path.o

TEST_OBJECTS =
TEST_OBJECTS += src/config.o
TEST_OBJECTS += src/path.o

include config.mk

.PHONY: all
all: $(BINOUT)/malachi $(BINOUT)/config_test

$(OBJECTS): $(HEADERS)

$(BINOUT):
	mkdir -p $@

$(BINOUT)/malachi: LDLIBS += -lsqlite3 -lmupdf -lgit2
$(BINOUT)/malachi: $(OBJECTS) | $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BINOUT)/config_test: $(TEST_OBJECTS) | $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(LDLIBS)

.c.o:
	$(CC) $(ALL_CFLAGS) -o $@ -c $<

.PHONY: check
check: $(BINOUT)/config_test
	$(BINOUT)/config_test

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json src/*.c

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(BINOUT)/malachi $(DESTDIR)$(bindir)/malachi

.PHONY: clean
clean:
	rm -rf -- $(BINOUT) $(OBJECTS) $(TEST_OBJECTS)
