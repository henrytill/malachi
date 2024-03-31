CFLAGS = -g -std=c11 -Iinclude -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
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

$(BINOUT)/malachi: LDLIBS += -lsqlite3 -lmupdf
$(BINOUT)/malachi: $(OBJECTS)
	@mkdir -p -- $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BINOUT)/config_test: $(TEST_OBJECTS)
	@mkdir -p -- $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(LDLIBS)

.PHONY: check
check: $(BINOUT)/config_test
	$(BINOUT)/config_test

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json src/*.c

.PHONY: clean
clean:
	rm -rf -- $(BINOUT) $(OBJECTS) $(TEST_OBJECTS)
