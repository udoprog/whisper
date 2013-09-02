SOURCES+=wsp.c
HEADERS+=wsp.h

CFLAGS=-g -Wall

all: whisper-dump

whisper-dump: $(SOURCES) $(HEADERS) whisper-dump.c
	$(CC) $(CFLAGS) -o whisper-dump $(SOURCES) whisper-dump.c
