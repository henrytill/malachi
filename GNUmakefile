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

MAIN_OBJECTS =
MAIN_OBJECTS += src/config.o
MAIN_OBJECTS += src/main.o
MAIN_OBJECTS += src/path.o

TEST_OBJECTS =
TEST_OBJECTS += src/config.o
TEST_OBJECTS += src/path.o

MAIN_CFLAGS = $(LIBGIT2_CFLAGS) $(MUPDF_CFLAGS) $(SQLITE3_CFLAGS)
MAIN_LIBS = $(LIBGIT2_LIBS) $(MUPDF_LIBS) $(SQLITE3_LIBS)

include config.mk

.PHONY: all
all: $(BINOUT)/malachi $(BINOUT)/config_test

src/main.o: ALL_CFLAGS += $(MAIN_CFLAGS)
src/main.o: src/main.c include/config.h include/error.h include/platform.h

src/config.o: src/config.c include/config.h include/error.h include/path.h include/platform.h

src/path.o: src/path.c include/path.h

$(BINOUT):
	mkdir -p $@

$(BINOUT)/malachi: LDLIBS += $(MAIN_LIBS)
$(BINOUT)/malachi: $(MAIN_OBJECTS) | $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(MAIN_OBJECTS) $(LDLIBS)

$(BINOUT)/config_test: $(TEST_OBJECTS) | $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(LDLIBS)

.c.o:
	$(CC) $(ALL_CFLAGS) -o $@ -c $<

.PHONY: check
check: $(BINOUT)/config_test
	$(BINOUT)/config_test

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json src/*.c test/*.c

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(BINOUT)/malachi $(DESTDIR)$(bindir)/malachi

.PHONY: clean
clean:
	rm -rf -- $(BINOUT) $(MAIN_OBJECTS) $(TEST_OBJECTS)
