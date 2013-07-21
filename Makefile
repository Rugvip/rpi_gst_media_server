CC=gcc

CFLAGS=-Wall `pkg-config --cflags glib-2.0`
LDFLAGS=`pkg-config --libs gio-2.0`

all: mediastreamer

mediastreamer: obj/main.o
	$(CC) $(LDFLAGS) -o mediastreamer obj/main.o

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

clean:
	rm -rf obj/*
