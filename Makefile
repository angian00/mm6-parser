CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=main.c parser.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mm6_parse.x

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

