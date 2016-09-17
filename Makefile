all:
	gcc -Wall --std=c99 -lm `pkg-config --libs opencv` main.c -o wavesim
