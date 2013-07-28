CC=gcc

CFLAGS=-Wall -Wextra -Werror -std=c11
LDFLAGS=

GLIB_CFLAGS=`pkg-config --cflags json-glib-1.0`
GST_CFLAGS=`pkg-config --cflags gstreamer-1.0`

LIBS=`pkg-config --libs gstreamer-1.0 json-glib-1.0`

OBJ=obj/main.o obj/pipeline.o obj/jsongen.o obj/jsonparse.o obj/jsonio.o

all: mediastreamer

mediastreamer: $(OBJ)
	$(CC) $(LDFLAGS) -o mediastreamer $(OBJ) $(LIBS)

obj/main.o: src/main.c src/pipeline.h
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c src/main.c -o obj/main.o

obj/pipeline.o: src/pipeline.c src/pipeline.h
	$(CC) $(CFLAGS) $(GST_CFLAGS) -c src/pipeline.c -o obj/pipeline.o

obj/jsongen.o: src/jsongen.c src/jsongen.h
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c src/jsongen.c -o obj/jsongen.o

obj/jsonparse.o: src/jsonparse.c src/jsonparse.h
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c src/jsonparse.c -o obj/jsonparse.o

obj/jsonio.o: src/jsonio.c src/jsonio.h
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c src/jsonio.c -o obj/jsonio.o

clean:
	rm -rf obj/*
