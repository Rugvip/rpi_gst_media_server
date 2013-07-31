CC=gcc

CFLAGS=-Wall -Wextra -Werror -std=c11
LDFLAGS=

CFLAGS += `pkg-config --cflags json-glib-1.0`
CFLAGS += `pkg-config --cflags gstreamer-1.0`

LIBS=`pkg-config --libs gstreamer-1.0 json-glib-1.0`

OBJ=obj/main.o obj/player.o obj/jsongen.o obj/jsonparse.o obj/jsonio.o \
	obj/handler.o obj/server.o

all: mediaserver

mediaserver: $(OBJ)
	$(CC) $(LDFLAGS) -o mediaserver $(OBJ) $(LIBS)

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/player.o: src/player.c src/player.h
	$(CC) $(CFLAGS) -c src/player.c -o obj/player.o

obj/jsongen.o: src/jsongen.c src/jsongen.h
	$(CC) $(CFLAGS) -c src/jsongen.c -o obj/jsongen.o

obj/jsonparse.o: src/jsonparse.c src/jsonparse.h
	$(CC) $(CFLAGS) -c src/jsonparse.c -o obj/jsonparse.o

obj/jsonio.o: src/jsonio.c src/jsonio.h
	$(CC) $(CFLAGS) -c src/jsonio.c -o obj/jsonio.o

obj/server.o: src/server.c src/server.h
	$(CC) $(CFLAGS) -c src/server.c -o obj/server.o

obj/handler.o: src/handler.c src/handler.h
	$(CC) $(CFLAGS) -c src/handler.c -o obj/handler.o

clean:
	rm -rf obj/*
