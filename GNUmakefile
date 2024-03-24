CFLAGS = -g -std=c11 -Iinclude -Wall -Wextra -Wconversion -Wsign-conversion
BINOUT = _bin

HEADERS =
HEADERS += include/config.h
HEADERS += include/path.h
HEADERS += include/platform.h

OBJECTS =
OBJECTS += src/config.o
OBJECTS += src/main.o
OBJECTS += src/path.o

TEST_OBJECTS =
TEST_OBJECTS += src/config.o
TEST_OBJECTS += src/path.o
TEST_OBJECTS += test/config_test.o

.PHONY: all
all: $(BINOUT)/malachi $(BINOUT)/config_test

$(OBJECTS): $(HEADERS)

$(BINOUT):
	mkdir -p -- $(BINOUT)

$(BINOUT)/malachi: LDLIBS += -lsqlite3 -lmupdf
$(BINOUT)/malachi: $(OBJECTS) $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BINOUT)/config_test: $(TEST_OBJECTS) $(BINOUT)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(LDLIBS)

.PHONY: check
check: $(BINOUT)/config_test
	$(BINOUT)/config_test

.PHONY: clean
clean:
	rm -rf -- $(BINOUT) $(OBJECTS) $(TEST_OBJECTS)