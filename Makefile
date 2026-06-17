all: points

points: points.c
	$(CC) -O2 points.c `sdl2-config --cflags` `sdl2-config --libs` -lm -o points -Wall -W

clean:
	rm -f points