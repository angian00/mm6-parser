CC = gcc
CFLAGS = -c -Wall -g -std=gnu99 -I/usr/local/include -I./src
LDFLAGS = -g -L/usr/local/lib

SRCDIR = src
OBJDIR = build

SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

#LIBS=-lGL -lGLEW -lSDL2 -lz
#LIBS=-lGLEW -lSDL2 -lz -framework OpenGL $on macOS
LIBS=-lglu32 -lglfw3 -lglew32 -lSDL2 -lz -lopengl32 -lpng
#LDLIBS=

EXECUTABLE=mm6_parse.x

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c | makedirs
	@echo
	@echo "--compiling"
	$(CC) $(CFLAGS) -c $< -o $@


# `|` is order-only-prerequisites
# https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html
makedirs:
	mkdir -p $(OBJDIR)

.PHONY: clean

clean:
	@echo
	@echo "--cleaning up"
	@echo
	rm -rf $(OBJDIR)
	rm -f $(EXECUTABLE)
