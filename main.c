#include <stdlib.h>
// #include <stdio.h>
// #include <math.h>
// #include <unistd.h> // For usleep
#include <time.h>

// #include "mathutils.h"
#include "display.h"
#include "sim.h"
#include "sources.h"
#include "sourcevalues.h"

int main(int argc, char *argv[]) {
    int width, height;
    width = 200;
    height = 200;
    if (argc == 3) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    World *world = world_init(width, height);
    if (world == NULL)
        return -1;

    display_init(world);

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

    Source source1 = PointSource(55, 80, SineWave(1.0/48.0, 4.0, 0.0));
    world_add_source(world, source1);

    // Clear screen, then initialise vbuf so it won't shake so much

    // printf("\033[2J");
    // stdoutsetvbuf(width*height*100);

    float fps = 30;
    clock_t nextframe = clock();

    // Display the initial state
    display_world(world);
    while (1) {
        // Tick and display state
        world_tick(world, 1);

        // Weird driving waves for fun and profit
        // frame_write(world->positions, 158, 153, 2.0f*sin((float)i*3.14159265358979/44)*sin((float)i*3.14159265358979/413)*cos((float)i*3.14159265358979/4859));
        // frame_write(world->positions, 27, 27, 4.0f*sin((float)i*3.14159265358979/24)*sin((float)i*3.14159265358979/300)*cos((float)i*3.14159265358979/4800));
        // frame_write(world->positions, 55, 80, 4.0f*sin((float)i*3.14159265358979/24));
        if (clock() > nextframe) {
            nextframe = clock() + CLOCKS_PER_SEC / fps;
            display_world(world);
        }
    }

    // Clean up and die
    world_kill(world);
    display_kill();

    return 0;
}
