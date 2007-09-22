SRC=clutter-layout.c


build:
	 gcc `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/ pkg-config --libs --cflags clutter-0.4` $(SRC)

run: build
	./a.out

edit:
	vim $(SRC)
