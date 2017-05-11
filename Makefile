CC=gcc
CFLAGS=-c -Wall -g -std=gnu99
LDFLAGS=-g
SOURCES=main.c parser.c
OBJECTS=$(SOURCES:.c=.o)
LIBS=-lz
#LDLIBS=
EXECUTABLE=mm6_parse.x

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

