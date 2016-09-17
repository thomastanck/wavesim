#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h> // For usleep
#include <time.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#define MAX_REACH 1
#define ZOOM 2
#define DAMP_SIZE 50

// Utils

float sigmoid(float x) {
    return 1 / (1+exp(-x));
}

// Already defined in opencv
// #define MAX(x, y) ((x) > (y) ? (x) : (y))
// #define MIN(x, y) ((x) < (y) ? (x) : (y))

// Structs

typedef struct Frame {
    int width, height;
    float values[];
} Frame;

typedef struct World {
    int width, height;

    Frame *accelerations;
    Frame *velocities;
    Frame *positions;

    Frame *dampaccelerations;
    Frame *dampvelocities;
    Frame *damppositions;
} World;

typedef struct RGB {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

// Frame shit

Frame *frame_init(int width, int height) {
    Frame *frame = (Frame *) calloc(sizeof(frame) + sizeof(float) * width * height, sizeof(char));
    if (frame == NULL)
        return frame;

    frame->width = width;
    frame->height = height;

    return frame;
}

void frame_kill(Frame *frame) {
    free(frame);
}

// 'safe' read/write functions.
int out_of_frame(Frame *frame, int x, int y) {
    return (x < 0) || (x >= frame->width) || (y < 0) || (y >= frame->height);
}
int is_boundary(Frame *frame, int x, int y) {
    return (x == 0) || (x == frame->width - 1) || (y == 0) || (y == frame->height - 1);
}
int dist_from_boundary(Frame *frame, int x, int y) {
    return MIN(MIN(x, frame->width - 1 - x), MIN(y, frame->height - 1 - y));
}

float frame_read(Frame *frame, int x, int y, float def) {
    if ((x < 0) || (x >= frame->width) || (y < 0) || (y >= frame->height))
        return def;

    return frame->values[frame->width * y + x];
}

void frame_write(Frame *frame, int x, int y, float value) {
    if ((x < 0) || (x >= frame->width) || (y < 0) || (y >= frame->height))
        return;

    frame->values[frame->width * y + x] = value;
}

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

void outputcell(RGB color) {
    printf("\033[48;2;%u;%u;%u;m  \033[0m", color.r, color.g, color.b);
}

CvMat *mat;

void displayframe(Frame *frame) {
    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0; x < frame->width; ++x) {
            for (int yz = 0; yz < ZOOM; ++yz) {
                for (int xz = 0; xz < ZOOM; ++xz) {
                    mat->data.ptr[(y*ZOOM+yz)*frame->width*ZOOM+x*ZOOM+xz] = (unsigned char) 255 * sigmoid(frame_read(frame, x, y, 0.0f));
                }
            }
        }
    }
    cvShowImage("wavesim", mat);
    cvWaitKey(1);
    return;
    // RGB start = newRGB(0, 0, 255);
    // RGB end = newRGB(255, 0, 0);
    // //printf("\033[2J");
    // printf("\033[%uA", frame->height + 1);
    // for (int y = 0; y < frame->height; ++y) {
    //     for (int x = 0; x < frame->width; ++x) {
    //         outputcell(interpolatecolor(start, end, 255 * sigmoid(frame_read(frame, x, y, 0.0f))));
    //     }
    //     printf("\n");
    // }
    // fflush(stdout);
}

void displayworld(World *world) {
    RGB start = newRGB(0, 0, 255);
    RGB end = newRGB(255, 0, 0);
    //printf("\033[2J");
    printf("\033[%uA", world->positions->height + 1);
    for (int y = 0; y < world->positions->height; ++y) {
        for (int x = 0; x < world->positions->width; ++x) {
            outputcell(interpolatecolor(start, end, 255 * sigmoid(frame_read(world->positions, x, y, 0.0f))));
        }
        printf(" ");
        for (int x = 0; x < world->velocities->width; ++x) {
            outputcell(interpolatecolor(start, end, 255 * sigmoid(frame_read(world->velocities, x, y, 0.0f) / 3)));
        }
        printf(" ");
        for (int x = 0; x < world->accelerations->width; ++x) {
            outputcell(interpolatecolor(start, end, 255 * sigmoid(frame_read(world->accelerations, x, y, 0.0f) / MAX_REACH / MAX_REACH / 3)));
        }
        printf("\n");
    }
    fflush(stdout);
}

// Actual Physics

void update_accelerations(Frame *pos, Frame *accels, Frame *dampaccels) {
    // for (int y = 0; y < accels->height; y++) {
    //     for (int x = 0; x < accels->width; x++) {

    //         float src = frame_read(pos, x, y, 0.0);

    //         float accel = 0;
    //         float norm = 0;
    //         for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
    //             for (int i = - MAX_REACH; i <= MAX_REACH; i++) {

    //                 if (i == 0 && j == 0)
    //                     continue;
    //                 if (out_of_frame(pos, x+i, y+j))
    //                     continue;

    //                 float distance = (i * i + j * j); // (L2 dist) ^ 2

    //                 float diff = frame_read(pos, x+i, y+j, 0.0) - src;
    //                 accel += diff / distance;
    //                 norm += distance;
    //             }
    //         }

    //         frame_write(accels, x, y, accel * norm);
    //     }
    // }
    for (int y = 0; y < accels->height; y++) {
        for (int x = 0; x < accels->width; x++) {
            frame_write(accels, x, y, 0);
        }
    }
    for (int y = 0; y < pos->height; y++) {
        for (int x = 0; x < pos->width; x++) {
            float src = frame_read(pos, x, y, 0.0);

            float norm = 0;
            for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
                for (int i = - MAX_REACH; i <= MAX_REACH; i++) {
                    if (i == 0 && j == 0)
                        continue;
                    if (out_of_frame(pos, x+i, y+j))
                        continue;
                    float distance = (i * i + j * j); // (L2 dist) ^ 2
                    norm += 1.0 / distance;
                }
            }
            for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
                for (int i = - MAX_REACH; i <= MAX_REACH; i++) {
                    if (i == 0 && j == 0)
                        continue;
                    if (out_of_frame(pos, x+i, y+j))
                        continue;
                    float distance = (i * i + j * j); // (L2 dist) ^ 2
                    float diff = frame_read(pos, x+i, y+j, 0.0) - src;
                    accels->values[accels->width * y + x] += diff / distance / norm;// * dampaccels->values[accels->width * (y+j) + (x+i)];
                    // int boundary_dist = dist_from_boundary(pos, x, y);
                    // boundary_dist = 4;
                    // if (boundary_dist <= 3) {
                    //     accels->values[accels->width * y + x] += diff / distance / norm * (boundary_dist + 1) / 4;
                    // } else {
                    //     accels->values[accels->width * y + x] += diff / distance / norm;
                    // }
                }
            }
            accels->values[accels->width * y + x] *= dampaccels->values[accels->width * y + x];
        }
    }
}

void update_velocities(Frame *accels, Frame *vels, Frame *dampvels, float delta) {

    for (int y = 0; y < vels->height; y++) {
        for (int x = 0; x < vels->width; x++) {
            int index = y * vels->width + x;
            vels->values[index] += delta * accels->values[index];
            vels->values[index] *= pow(dampvels->values[index], delta);
        }
    }
}

void update_positions(Frame *vels, Frame *pos, Frame *damppos, float delta) {

    for (int y = 0; y < pos->height; y++) {
        for (int x = 0; x < pos->width; x++) {
            int index = y * pos->width + x;
            pos->values[index] += delta * vels->values[index];
            pos->values[index] *= pow(damppos->values[index], delta);
        }
    }
}

// World shit

void world_kill(World *world) {

    if (world->accelerations) frame_kill(world->accelerations);
    if (world->velocities) frame_kill(world->velocities);
    if (world->positions) frame_kill(world->positions);
    free(world);
}

World *world_init(int width, int height) {

    World *world = (World *) malloc(sizeof(world));
    if (world == NULL)
        return world;

    world->width = width;
    world->height = height;

    world->accelerations = frame_init(width, height);
    world->velocities = frame_init(width, height);
    world->positions = frame_init(width, height);

    world->dampaccelerations = frame_init(width, height);
    world->dampvelocities = frame_init(width, height);
    world->damppositions = frame_init(width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            world->dampaccelerations->values[index] = 1; // walls/refraction
            world->dampvelocities->values[index] = 1;
            world->damppositions->values[index] = 1; // walls

            // Boundary damping
            int boundary_dist = dist_from_boundary(world->dampvelocities, x, y);
            // boundary_dist = DAMP_SIZE + 1; // Disable damping
            if (boundary_dist <= DAMP_SIZE) {
#define CUBE(x) (x) * (x) * (x)
                world->dampvelocities->values[index] = 1 - CUBE((float)(DAMP_SIZE - boundary_dist) / DAMP_SIZE);
#undef CUBE
            }

            // Lloyd's mirror
            if (y == 50) {
                world->damppositions->values[index] = 0;
            }

            // Top fourth is an "optic fibre"
            // if (y > 1 * height / 4 - 5 && y < 1 * height / 4 + 15) {
            //     world->dampaccelerations->values[index] = 0.3;
            // }
            // if (x > 3 * width / 4) {
            //     world->dampaccelerations->values[index] = 1;
            // }

            // if (x > width / 3 && x < 2 * width / 3) {
            //     world->damppositions->values[index] = 0;
            // }
            // if (x > 2 * width / 5 && x < 3 * width / 5) {
            //     world->damppositions->values[index] = 1;
            // }
            // if (y > 1 * height / 4 - 5 && y < 1 * height / 4 + 15) {
            //     world->damppositions->values[index] = 1;
            // }

            // Wall at right third
            // if (x == 2 * width / 3) {
            //     world->damppositions->values[index] = 0;
            // }
            // Unless at the third points...
            // if (abs(y - height / 3) < 2 || abs(y - 2 * height / 3) < 2) {
            //     world->damppositions->values[index] = 1;
            // }

            // Scatter at bottom left
            // if (x == width / 3 && y == 3 * height / 4) {
            //     world->damppositions->values[index] = 0;
            // }
        }
    }

    if ((world->accelerations == NULL) ||
        (world->velocities == NULL) ||
        (world->positions == NULL)) {
        world_kill(world);
        return NULL;
    }

    return world;
}

void world_tick(World *world, float delta) {

    update_accelerations(world->positions, world->accelerations, world->dampaccelerations);
    update_velocities(world->accelerations, world->velocities, world->dampvelocities, delta);
    update_positions(world->velocities, world->positions, world->damppositions, delta);
}

void print_world(World *world) {
    displayframe(world->positions);
    // printf("\n");
}

int main(int argc, char *argv[]) {
    int width, height;
    width = 200;
    height = 200;
    if (argc == 3) {
        sscanf(argv[1], "%d", &width);
        sscanf(argv[2], "%d", &height);
    }

    // Window stuff
    mat = cvCreateMat(height*ZOOM, width*ZOOM, CV_8UC1);
    cvNamedWindow("wavesim", CV_WINDOW_AUTOSIZE); // Create a window for display.

    World *world = world_init(width, height);
    if (world == NULL)
        return -1;

    // Initialise the state here!

    // Nice wave around 20, 10
    // for (int i = -6; i <= 6; ++i) {
    //     for (int j = -6; j <= 6; ++j) {
    //         // if (i == 0 && j == 0)
    //         //     continue;
    //         frame_write(world->positions, 100+i, 100+j, 3*(1+cos(MAX(-3.14159265358979, -sqrt(i * i + j * j)/2))));
    //     }
    // }

    // Wall of wave at x=15
    // for (int i = -6; i <= 6; ++i) {
    //     for (int y = 0; y <= 54; ++y) {
    //         // if (i == 0 && j == 0)
    //         //     continue;
    //         frame_write(world->positions, 15+i, y, 2*(1+cos(MAX(-3.14159265358979, -i/2))));
    //     }
    // }

    // Two boxes of 2.0f around 12, 17 and 12, 37
    // for (int x = 10; x <= 14; x++) {
    //     for (int y = 15; y <= 19; y++) {
    //         frame_write(world->positions, x, y, 2.0f);
    //     }
    // }
    // for (int x = 10; x <= 14; x++) {
    //     for (int y = 35; y <= 39; y++) {
    //         frame_write(world->positions, x, y, 2.0f);
    //     }
    // }

    // Clear screen, then initialise vbuf so it won't shake so much

    // printf("\033[2J");
    // stdoutsetvbuf(width*height*100);

    float fps = 30;
    clock_t nextframe = clock();

    // Display the initial state
    print_world(world);
    for (int i = 0; i < 1000000; i++) {
        // Tick and display state
        world_tick(world, 1);

        // Weird driving waves for fun and profit
        // frame_write(world->positions, 158, 153, 2.0f*sin((float)i*3.14159265358979/44)*sin((float)i*3.14159265358979/413)*cos((float)i*3.14159265358979/4859));
        // frame_write(world->positions, 27, 27, 4.0f*sin((float)i*3.14159265358979/24)*sin((float)i*3.14159265358979/300)*cos((float)i*3.14159265358979/4800));
        frame_write(world->positions, 55, 80, 4.0f*sin((float)i*3.14159265358979/24));
        if (clock() > nextframe) {
            nextframe = clock() + CLOCKS_PER_SEC / fps;
            print_world(world);
        }
    }

    // Clean up and die
    world_kill(world);
    cvReleaseMat(&mat);

    return 0;
}
