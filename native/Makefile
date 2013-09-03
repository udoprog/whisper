SOURCES+=src/wsp.c
SOURCES+=src/wsp_io_file.c
SOURCES+=src/wsp_io_mmap.c
OBJECTS=$(SOURCES:.c=.o)
ARCHIVE=wsp.a

TESTS+=tests/test_wsp_io_file.test

CFLAGS=-g -pedantic -Wall -O3 -std=gnu11

all: whisper-dump

clean:
	$(RM) $(OBJECTS)
	$(RM) $(ARCHIVE)
	$(RM) whisper-dump

%.test: %.o
	$(CC) $< $(shell pkg-config --libs check) -o $@

.PHONY: tests

tests: $(TESTS)
	@for test in $(TESTS); do echo "TEST: $$test"; $$test; done

$(ARCHIVE): $(OBJECTS)
	$(AR) cr $@ $(OBJECTS)

whisper-dump: src/whisper-dump.o $(ARCHIVE)
	$(CC) $(CFLAGS) -o whisper-dump src/whisper-dump.o $(ARCHIVE)
