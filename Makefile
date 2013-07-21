CC=gcc

CFLAGS=-Wall -Werror -std=c11
LDFLAGS=

GLIB_CFLAGS=`pkg-config --cflags json-glib-1.0`
GST_CFLAGS=`pkg-config --cflags gstreamer-1.0`

LIBS=`pkg-config --libs gstreamer-1.0 json-glib-1.0`

all: mediastreamer

mediastreamer: obj/main.o obj/pipeline.o
	$(CC) $(LDFLAGS) -o mediastreamer obj/pipeline.o obj/main.o $(LIBS)

obj/main.o: src/main.c src/pipeline.h
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c src/main.c -o obj/main.o

obj/pipeline.o: src/pipeline.c src/pipeline.h
	$(CC) $(CFLAGS) $(GST_CFLAGS) -c src/pipeline.c -o obj/pipeline.o

clean:
	rm -rf obj/*
