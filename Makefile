all:
	gcc -Wall --std=c99 -lm `pkg-config --libs opencv` main.c display.c sim.c sources.c sourcevalues.c -o wavesim
