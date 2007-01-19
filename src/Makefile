CC = g++ -Wall -ansi

CFLAGS = `sdl-config --cflags`
LIBS = -lGL -lGLU `sdl-config --libs` -lSDL_image

all:
	$(CC) trilogy.cc list_files.cc -o trilogy $(CFLAGS) $(LIBS)

clean:
	@echo Cleaning up...
	@rm trilogy
	@echo Done.

tags:
	ctags *.h *.c
