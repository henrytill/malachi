CXXFLAGS = -g -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
ALL_CXXFLAGS = -std=c++20 -Iinclude $(CXXFLAGS)

LDFLAGS =
ALL_LDFLAGS = $(LDFLAGS)

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
TEST_OBJECTS += src/config.o
TEST_OBJECTS += test/platform_test.o
TEST_OBJECTS += test/config_test.o

TEST_EXES =
TEST_EXES += test/platform_test
TEST_EXES += test/config_test

MAIN_CFLAGS = $(LIBGIT2_CFLAGS) $(MUPDF_CFLAGS) $(SQLITE3_CFLAGS)
MAIN_LIBS = $(LIBGIT2_LIBS) $(MUPDF_LIBS) $(SQLITE3_LIBS)

include config.mk

.PHONY: all
all: $(MAIN_EXES) $(TEST_EXES)

src/main.o: ALL_CXXFLAGS += $(MAIN_CFLAGS)
src/main.o: src/main.cpp include/config.h include/platform.h

src/config.o: src/config.cpp include/config.h include/platform.h

src/main: LDLIBS += $(MAIN_LIBS)
src/main: $(MAIN_OBJECTS)
	$(CXX) $(ALL_LDFLAGS) -o $@ $(MAIN_OBJECTS) $(LDLIBS)

test/config_test: src/config.o test/config_test.o
	$(CXX) $(ALL_LDFLAGS) -o $@ $^ $(LDLIBS)

test/platform_test: test/platform_test.o
	$(CXX) $(ALL_LDFLAGS) -o $@ $^ $(LDLIBS)

.cpp.o:
	$(CXX) $(ALL_CXXFLAGS) -o $@ -c $<

.PHONY: check
check: $(TEST_EXES)
	./test/config_test
	./test/platform_test

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
