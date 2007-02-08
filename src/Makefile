CC = g++ -Wall -ansi -ggdb

CFLAGS = `sdl-config --cflags` `pkg-config --cflags librsvg-2.0`
LIBS = -lGL -lGLU `sdl-config --libs` -lSDL_image `pkg-config --libs librsvg-2.0`

all:
	$(CC) trilogy.cc list_files.cc -o trilogy $(CFLAGS) $(LIBS)

dnd:
	$(CC) dnd.cc -o dnd `pkg-config --cflags --libs gtk+-2.0`

clean:
	@echo Cleaning up...
	@rm trilogy
	@echo Done.

tags:
	ctags *.h *.c
