// #define DISPLAY_USING_ANSI
#define DISPLAY_USING_WINDOW

#ifdef DISPLAY_USING_ANSI
#include <stdlib.h>
#include <stdio.h>
#endif
#ifdef DISPLAY_USING_WINDOW
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

#include "display.h"
#include "mathutils.h"

#define ZOOM 2

#ifdef DISPLAY_USING_ANSI
typedef struct RGB RGB;

struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

// RGB shit

RGB newRGB(unsigned char r, unsigned char g, unsigned char b) {
	RGB color;
	color.r = r;
	color.g = g;
	color.b = b;
	return color;
}

RGB interpolatecolor(RGB start, RGB end, unsigned char val) {
	RGB color;
	color.r = ((unsigned)start.r * (255-(unsigned)val) + (unsigned)end.r * (unsigned)val) / 255;
	color.g = ((unsigned)start.g * (255-(unsigned)val) + (unsigned)end.g * (unsigned)val) / 255;
	color.b = ((unsigned)start.b * (255-(unsigned)val) + (unsigned)end.b * (unsigned)val) / 255;
	return color;
}

void outputcolor(RGB color) {
	printf("%u,%u,%u\n", color.r, color.g, color.b);
}

// Display shit

static char *vbuf = NULL;
void stdoutsetvbuf(size_t n) {
	vbuf = realloc(vbuf, n);
	setvbuf(stdout, vbuf, _IOFBF, n);
}

void ANSICell(RGB color) {
	printf("\033[48;2;%u;%u;%u;m  \033[0m", color.r, color.g, color.b);
}

void ANSIFrame(Frame *frame) {
	RGB start = newRGB(0, 0, 255);
	RGB end = newRGB(255, 0, 0);
	printf("\033[%uA", frame->height + 1);
	for (int y = 0; y < frame->height; ++y) {
		for (int x = 0; x < frame->width; ++x) {
			ANSICell(interpolatecolor(start, end, 255 * SIGMOID(frame_read(frame, x, y, 0.0f))));
		}
		printf("\n");
	}
	printf("\n");
	fflush(stdout);
}

void ANSIWorld(World *world) {
	RGB start = newRGB(0, 0, 255);
	RGB end = newRGB(255, 0, 0);
	printf("\033[%uA", world->positions->height + 1);
	for (int y = 0; y < world->positions->height; ++y) {
		for (int x = 0; x < world->positions->width; ++x) {
			ANSICell(interpolatecolor(start, end, 255 * SIGMOID(frame_read(world->positions, x, y, 0.0f))));
		}
		printf(" ");
		for (int x = 0; x < world->velocities->width; ++x) {
			ANSICell(interpolatecolor(start, end, 255 * SIGMOID(frame_read(world->velocities, x, y, 0.0f) / 3)));
		}
		printf(" ");
		for (int x = 0; x < world->accelerations->width; ++x) {
			ANSICell(interpolatecolor(start, end, 255 * SIGMOID(frame_read(world->accelerations, x, y, 0.0f) / MAX_REACH / MAX_REACH / 3)));
		}
		printf("\n");
	}
	printf("\n");
	fflush(stdout);
}
#endif

#ifdef DISPLAY_USING_WINDOW
static CvMat *mat = NULL;

void windowFrame(Frame *frame) {
	for (int y = 0; y < frame->height; ++y) {
		for (int x = 0; x < frame->width; ++x) {
			for (int yz = 0; yz < ZOOM; ++yz) {
				for (int xz = 0; xz < ZOOM; ++xz) {
					mat->data.ptr[(y*ZOOM+yz)*frame->width*ZOOM+x*ZOOM+xz] = (unsigned char) 255 * SIGMOID(frame_read(frame, x, y, 0.0f));
				}
			}
		}
	}
	cvShowImage("wavesim", mat);
	cvWaitKey(1);
}
#endif

void display_init(World *world) {
#ifdef DISPLAY_USING_ANSI
	setvbuf(world->width * world->height * 50);
#endif
#ifdef DISPLAY_USING_WINDOW
	// Window stuff
	mat = cvCreateMat(world->height*ZOOM, world->width*ZOOM, CV_8UC1);
	cvNamedWindow("wavesim", CV_WINDOW_AUTOSIZE); // Create a window for display.
#endif
}

void display_world(World *world) {
#ifdef DISPLAY_USING_ANSI
	ANSIFrame(world->positions);
#endif
#ifdef DISPLAY_USING_WINDOW
	windowFrame(world->positions);
#endif
}

void display_kill() {
#ifdef DISPLAY_USING_WINDOW
	cvReleaseMat(&mat);
#endif
}

