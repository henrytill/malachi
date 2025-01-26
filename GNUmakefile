CXXFLAGS = -g -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
ALL_CXXFLAGS = -std=c++20 -Iinclude $(CXXFLAGS)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

bindir = /bin
prefix = /usr/local
DESTDIR = $(prefix)

HEADERS =
HEADERS += include/config.h
HEADERS += include/platform.h

MAIN_OBJECTS =
MAIN_OBJECTS += src/config.o
MAIN_OBJECTS += src/main.o

MAIN_EXES =
MAIN_EXES += src/main

TEST_OBJECTS =

TEST_EXES =

MAIN_CFLAGS = $(LIBGIT2_CFLAGS) $(MUPDF_CFLAGS) $(SQLITE3_CFLAGS)
MAIN_LIBS = $(LIBGIT2_LIBS) $(MUPDF_LIBS) $(SQLITE3_LIBS)

include config.mk

.PHONY: all
all: $(MAIN_EXES)

src/main.o: ALL_CXXFLAGS += $(MAIN_CFLAGS)
src/main.o: src/main.cpp include/config.h include/platform.h

src/config.o: src/config.cpp include/config.h include/platform.h

src/main: LDLIBS += $(MAIN_LIBS)
src/main: $(MAIN_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(MAIN_OBJECTS) $(LDLIBS)

.cpp.o:
	$(CXX) $(ALL_CXXFLAGS) -o $@ -c $<

.PHONY: check
check: $(TEST_EXES)
	@echo "not yet"

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json include/*.h src/*.cpp

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) src/main $(DESTDIR)$(bindir)/malachi

.PHONY: clean
clean:
	rm -rf -- $(MAIN_OBJECTS) $(TEST_OBJECTS) $(MAIN_EXES) $(TEST_EXES)
