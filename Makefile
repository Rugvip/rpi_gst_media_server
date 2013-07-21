CC=gcc

CFLAGS=-Wall -std=c11 `pkg-config --cflags json-glib-1.0`
LDFLAGS=`pkg-config --libs json-glib-1.0`

all: mediastreamer

mediastreamer: obj/main.o
	$(CC) $(LDFLAGS) -o mediastreamer obj/main.o

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

clean:
	rm -rf obj/*
