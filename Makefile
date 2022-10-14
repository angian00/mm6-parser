CC=gcc
CFLAGS=-c -Wall -g -std=gnu99 -I/usr/local/include
LDFLAGS=-g -L/usr/local/lib
SOURCES=main.c parser.c graphics.c geometry.c
OBJECTS=$(SOURCES:.c=.o)

#LIBS=-lGL -lGLEW -lSDL2 -lz
#LIBS=-lGLEW -lSDL2 -lz -framework OpenGL $on macOS
LIBS=-lglu32 -lglfw3 -lglew32 -lSDL2 -lz -lopengl32
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

