CC=gcc

CFLAGS=-Wall -Werror -std=c11
LDFLAGS=

GLIB_CFLAGS=`pkg-config --cflags json-glib-1.0`
GLIB_LIBS=`pkg-config --cflags json-glib-1.0`

GST_CFLAGS=`pkg-config --cflags gstreamer-1.0`
GST_LIBS=`pkg-config --libs gstreamer-1.0`


all: mediastreamer pipeliner

mediastreamer: obj/main.o
	$(CC) $(LDFLAGS) $(GLIB_LIBS) -o mediastreamer obj/main.o

obj/main.o: src/main.c
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c src/main.c -o obj/main.o

pipeliner: obj/pipeline.o
	$(CC) $(LDFLAGS) $(GST_LIBS) -o pipeliner obj/pipeline.o

obj/pipeline.o: src/pipeline.c
	$(CC) $(CFLAGS) $(GST_CFLAGS) -c src/pipeline.c -o obj/pipeline.o

clean:
	rm -rf obj/*
