SOURCES+=src/wsp.c
SOURCES+=src/wsp_private.c
SOURCES+=src/wsp_time.c
SOURCES+=src/wsp_io_file.c
SOURCES+=src/wsp_io_mmap.c
OBJECTS=$(SOURCES:.c=.o)
ARCHIVE=wsp.a

TESTS+=tests/test_wsp_io_file.1.test

CFLAGS=-g -pedantic -Wall -O3 -std=c99 -fPIC -D_POSIX_C_SOURCE

all: whisper-dump python-bindings

clean:
	$(RM) $(OBJECTS)
	$(RM) $(ARCHIVE)
	$(RM) whisper-dump
	$(RM) -R build

%.test: %.o
	$(CC) $< $(shell pkg-config --libs check) -o $@

.PHONY: tests

tests: $(TESTS)
	@for test in $(TESTS); do echo "TEST: $$test"; $$test; done

.PHONY: python-bindings

python-bindings:
	python setup.py build

$(ARCHIVE): $(OBJECTS)
	$(AR) cr $@ $(OBJECTS)

whisper-dump: src/whisper-dump.o $(ARCHIVE)
	$(CC) $(CFLAGS) -o whisper-dump src/whisper-dump.o $(ARCHIVE)
