#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#define MAX_REACH 1

float sigmoid(float x) {

    return 1 / ( 1+exp(-x));
}

struct frame {

    int width, height;
    float values[];
};

struct frame *frame_init(int width, int height) {

    struct frame *frame = (struct frame *) calloc(sizeof(struct frame) + sizeof(float) * width * height, sizeof(char));
    if (frame == NULL)
        return frame;

    frame->width = width;
    frame->height = height;

    return frame;
}

void frame_kill(struct frame *frame) {
    free(frame);
}

// 'safe' read/write functions.
int out_of_frame(struct frame *frame, int x, int y) {
    return (x < 0) || (x >= frame->width) || (y < 0) || (y >= frame->height);
}

float frame_read(struct frame *frame, int x, int y, float def) {

    if ((x < 0) || (x >= frame->width) || (y < 0) || (y >= frame->height))
        return def;

    return frame->values[frame->width * y + x];
}

void frame_write(struct frame *frame, int x, int y, float value) {

    if ((x < 0) || (x >= frame->width) || (y < 0) || (y >= frame->height))
        return;

    frame->values[frame->width * y + x] = value;
}

static char *vbuf = NULL;

struct RGB {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct RGB newRGB(unsigned char r, unsigned char g, unsigned char b) {
    struct RGB color;
    color.r = r;
    color.g = g;
    color.b = b;
    return color;
}

struct RGB interpolatecolor(struct RGB start, struct RGB end, unsigned char val) {
    struct RGB color;
    color.r = ((unsigned)start.r * (255-(unsigned)val) + (unsigned)end.r * (unsigned)val) / 255;
    color.g = ((unsigned)start.g * (255-(unsigned)val) + (unsigned)end.g * (unsigned)val) / 255;
    color.b = ((unsigned)start.b * (255-(unsigned)val) + (unsigned)end.b * (unsigned)val) / 255;
    return color;
}

void outputcolor(struct RGB color) {
    printf("%u,%u,%u\n", color.r, color.g, color.b);
}

void stdoutsetvbuf(size_t n) {
    vbuf = realloc(vbuf, n);
    setvbuf(stdout, vbuf, _IOFBF, n);
}

void outputcell(struct RGB color) {
    printf("\033[48;2;%u;%u;%u;m  \033[0m", color.r, color.g, color.b);
}

void testdisplay() {
    printf("\033[48;2;0;0;255;m  \033[0m\n");
    printf("\033[48;2;64;0;191;m  \033[0m\n");
    printf("\033[48;2;128;0;127;m  \033[0m\n");
    printf("\033[48;2;192;0;63;m  \033[0m\n");
    printf("\033[48;2;255;0;0;m  \033[0m\n");
    printf("\n");
    struct RGB start = newRGB(0, 0, 255);
    struct RGB end = newRGB(255, 0, 0);
    outputcell(interpolatecolor(start, end, 0));
    outputcell(interpolatecolor(start, end, 64));
    outputcell(interpolatecolor(start, end, 128));
    outputcell(interpolatecolor(start, end, 192));
    outputcell(interpolatecolor(start, end, 255));
    printf("\n");
    outputcell(interpolatecolor(start, end, 255));
    outputcell(interpolatecolor(start, end, 192));
    outputcell(interpolatecolor(start, end, 127));
    outputcell(interpolatecolor(start, end, 64));
    outputcell(interpolatecolor(start, end, 0));
    printf("\n");
    fflush(stdout);
}

void displayframe(struct frame *frame) {
    struct RGB start = newRGB(0, 0, 255);
    struct RGB end = newRGB(255, 0, 0);
    //printf("\033[2J");
    printf("\033[%uA", frame->height + 1);
    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0; x < frame->width; ++x) {
            outputcell(interpolatecolor(start, end, 255 * frame_read(frame, x, y, 0.0f)));
        }
        printf("\n");
    }
    fflush(stdout);
}

struct world {

    int width, height;

    struct frame *accelerations;
    struct frame *velocities;
    struct frame *positions;

    struct frame *output;
};

void displayworld(struct world *world) {
    struct RGB start = newRGB(0, 0, 255);
    struct RGB end = newRGB(255, 0, 0);
    //printf("\033[2J");
    printf("\033[%uA", world->output->height + 1);
    for (int y = 0; y < world->output->height; ++y) {
        for (int x = 0; x < world->output->width; ++x) {
            outputcell(interpolatecolor(start, end, 255 * frame_read(world->output, x, y, 0.0f)));
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

void displaysigmoidframe(struct frame *frame) {
    struct RGB start = newRGB(0, 0, 255);
    struct RGB end = newRGB(255, 0, 0);
    //printf("\033[2J");
    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0; x < frame->width; ++x) {
            outputcell(interpolatecolor(start, end, 255 * sigmoid(frame_read(frame, x, y, 0.0f))));
        }
        printf("\n");
    }
    fflush(stdout);
}

void world_kill(struct world *world) {

    if (world->accelerations) frame_kill(world->accelerations);
    if (world->velocities) frame_kill(world->velocities);
    if (world->positions) frame_kill(world->positions);
    if (world->output) frame_kill(world->output);
    free(world);
}

struct world *world_init(int width, int height) {

    struct world *world = (struct world *) malloc(sizeof(struct world));
    if (world == NULL)
        return world;

    world->width = width;
    world->height = height;

    world->accelerations = frame_init(width, height);
    world->velocities = frame_init(width, height);
    world->positions = frame_init(width, height);
    world->output = frame_init(width, height);

    if ((world->accelerations == NULL) ||
        (world->velocities == NULL) ||
        (world->positions == NULL) ||
        (world->output == NULL)) {
        world_kill(world);
        return NULL;
    }

    return world;
}

void update_output(struct frame *pos, struct frame *output) {

    for (int y = 0; y < output->height; y++) {
        for (int x = 0; x < output->width; x++) {
            int index = y * output->width + x;
            output->values[index] = sigmoid(pos->values[index]);
        }
    }
}

void update_accelerations(struct frame *pos, struct frame *accels) {
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
                    accels->values[accels->width * y + x] += diff / distance / norm;
                }
            }
        }
    }
}

void update_velocities(struct frame *accels, struct frame *vels, float delta) {

    for (int y = 0; y < vels->height; y++) {
        for (int x = 0; x < vels->width; x++) {
            int index = y * vels->width + x;
            vels->values[index] += delta * accels->values[index];
        }
    }
}

void update_positions(struct frame *vels, struct frame *pos, float delta) {

    for (int y = 0; y < pos->height; y++) {
        for (int x = 0; x < pos->width; x++) {
            int index = y * pos->width + x;
            pos->values[index] += delta * vels->values[index];
        }
    }
}

void world_tick(struct world *world, float delta) {

    update_accelerations(world->positions, world->accelerations);
    update_velocities(world->accelerations, world->velocities, delta);
    update_positions(world->velocities, world->positions, delta);
    update_output(world->positions, world->output);
}

struct frame *world_output(struct world *world) {
    return world->output;
}

void print_world(struct world *world) {

    //struct frame *output = world_output(world);
    // TODO: gracey
    // displaysigmoidframe(world->accelerations);
    // displaysigmoidframe(world->velocities);
    // displayframe(output);
    displayframe(world->output);
    printf("\n");
    // struct frame *output = world_output(world);
    // for (int y = 0; y < output->height; y++) {
    //     for (int x = 0; x < output->width; x++) {
    //         int index = y * output->width + x;
    //         float value = output->values[index];
    //         if (value < 0.2)
    //             putc(' ', stdout);
    //         else if (value < 0.4)
    //             putc('.', stdout);
    //         else if (value < 0.6)
    //             putc('o', stdout);
    //         else if (value < 0.8)
    //             putc('O', stdout);
    //         else
    //             putc('@', stdout);
    //     }
    //     putc('\n', stdout);
    // }
    // putc('\n', stdout);
}

float max(float x, float y) {
    if (x > y) {
        return x;
    } else {
        return y;
    }
}

int main(int argc, char *argv[]) {

    struct world *world = world_init(119, 55);
    if (world == NULL)
        return -1;

    for (int i = -6; i <= 6; ++i) {
        for (int j = -6; j <= 6; ++j) {
            // if (i == 0 && j == 0)
            //     continue;
            frame_write(world->positions, 15+i, 27+j, 3*(1+cos(max(-3.14159265358979, -sqrt(i * i + j * j)/2))));
        }
    }

    // for (int i = -6; i <= 6; ++i) {
    //     for (int y = 0; y <= 54; ++y) {
    //         // if (i == 0 && j == 0)
    //         //     continue;
    //         frame_write(world->positions, 15+i, y, 2*(1+cos(max(-3.14159265358979, -i/2))));
    //     }
    // }

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
    //world->positions->values[0] = 100;

    printf("\033[2J");
    stdoutsetvbuf(50*11*100);

    update_output(world->positions, world->output);
    print_world(world);
    for (int i = 0; i < 1000000; i++) {
        world_tick(world, 1);
        print_world(world);
        //usleep(30 * 1000);
    }

    world_kill(world);

    return 0;
}