# set the compiler
CC = gcc
LINKER = gcc
RM = rm -f

SDL_FLAGS += `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

CFLAGS = -g -Wall
CFLAGS += $(SDL_FLAGS)

LFLAGS = -g -Wall
LFLAGS += $(SDL_FLAGS)

EXEC = hunter

SRCDIR = src
OBJDIR = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

$(EXEC): $(OBJECTS)
	$(LINKER) $(LFLAGS) $(OBJECTS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./$(EXEC)

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	rmdir $(OBJDIR)

.PHONY: remove
remove: clean
	$(RM) $(EXEC)
