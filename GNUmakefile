CFLAGS = -g -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
ALL_CFLAGS = -std=c11 -Iinclude $(CFLAGS)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

bindir = /bin
prefix = /usr/local
DESTDIR = $(prefix)

HEADERS =
HEADERS += include/config.h
HEADERS += include/error.h
HEADERS += include/path.h
HEADERS += include/platform.h

MAIN_OBJECTS =
MAIN_OBJECTS += src/config.o
MAIN_OBJECTS += src/main.o
MAIN_OBJECTS += src/path.o

MAIN_EXES =
MAIN_EXES += src/main

TEST_OBJECTS =
TEST_OBJECTS += src/config.o
TEST_OBJECTS += src/path.o

TEST_EXES =
TEST_EXES += test/config_test

MAIN_CFLAGS = $(LIBGIT2_CFLAGS) $(MUPDF_CFLAGS) $(SQLITE3_CFLAGS)
MAIN_LIBS = $(LIBGIT2_LIBS) $(MUPDF_LIBS) $(SQLITE3_LIBS)

include config.mk

.PHONY: all
all: $(MAIN_EXES) $(TEST_EXES)

src/main.o: ALL_CFLAGS += $(MAIN_CFLAGS)
src/main.o: src/main.c include/config.h include/error.h include/platform.h

src/config.o: src/config.c include/config.h include/error.h include/path.h include/platform.h

src/path.o: src/path.c include/path.h

src/main: LDLIBS += $(MAIN_LIBS)
src/main: $(MAIN_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(MAIN_OBJECTS) $(LDLIBS)

test/config_test: $(TEST_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(LDLIBS)

.c.o:
	$(CC) $(ALL_CFLAGS) -o $@ -c $<

.PHONY: check
check: $(TEST_EXES)
	test/config_test

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json src/*.c test/*.c

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) src/main $(DESTDIR)$(bindir)/malachi

.PHONY: clean
clean:
	rm -rf -- $(MAIN_OBJECTS) $(TEST_OBJECTS) $(MAIN_EXES) $(TEST_EXES)
