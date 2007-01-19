CC = g++ -Wall -ansi

CFLAGS = `sdl-config --cflags`
LIBS = -lGL -lGLU `sdl-config --libs` -lSDL_image

all:
	$(CC) lesson06.c -o lesson06 $(CFLAGS) $(LIBS)

clean:
	@echo Cleaning up...
	@rm lesson06
	@echo Done.

tags:
	ctags *.h *.c
