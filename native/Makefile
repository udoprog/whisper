SOURCES+=src/wsp.c
SOURCES+=src/wsp_io_file.c
SOURCES+=src/wsp_io_mmap.c
OBJECTS=$(SOURCES:.c=.o)
ARCHIVE=wsp.a

CFLAGS=-g -pedantic -Wall -O3 -std=gnu11

all: whisper-dump

clean:
	$(RM) $(OBJECTS)
	$(RM) $(ARCHIVE)
	$(RM) whisper-dump

$(ARCHIVE): $(OBJECTS)
	$(AR) cr $@ $(OBJECTS)

whisper-dump: src/whisper-dump.o $(ARCHIVE)
	$(CC) $(CFLAGS) -o whisper-dump src/whisper-dump.o $(ARCHIVE)
