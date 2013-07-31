CC=gcc

CFLAGS=-Wall -Wextra -Werror -std=c11
LDFLAGS=

CFLAGS += `pkg-config --cflags json-glib-1.0`
CFLAGS += `pkg-config --cflags gstreamer-1.0`

LIBS=`pkg-config --libs gstreamer-1.0 json-glib-1.0`

_OBJ=main.o player.o jsongen.o jsonparse.o jsonio.o handler.o server.o
OBJ=$(patsubst %,obj/%,$(_OBJ))

$(info $(OBJ))

_HEADERS=common.h player.h jsongen.h jsonparse.h jsonio.h handler.h server.h
HEADERS=$(patsubst %,src/%,$(_HEADERS))

#HEADERS=`find src -name *.c | sed "s:c$$:h:"`
#OBJ=`find src -name *.c | sed -r "s:src/(.*)c$$:obj/\1o:"`

all: mediaserver

mediaserver: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

obj/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf obj/*
