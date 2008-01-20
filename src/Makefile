SRC=trilogy.cc MediaLoader.cc MediaItem.cc csvparser.cc


build:
	 g++ -ggdb `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/ pkg-config --libs --cflags clutter-0.4` $(SRC) -o trilogy

run: build
	./trilogy

edit:
	vim $(SRC)
