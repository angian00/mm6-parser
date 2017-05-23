CC=gcc
CFLAGS=-c -Wall -g -std=gnu99 -I/usr/include/SDL2
LDFLAGS=-g
SOURCES=main.c parser.c graphics.c geometry.c
OBJECTS=$(SOURCES:.c=.o)
LIBS=-lGL -lGLEW -lSDL2 -lz
#LDLIBS=
EXECUTABLE=mm6_parse.x

all: $(SOURCES) $(EXECUTABLE) $()
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

