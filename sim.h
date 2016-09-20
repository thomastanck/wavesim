#ifndef WAVESIM_SIM_H_
#define WAVESIM_SIM_H_

typedef struct World World;
typedef struct Frame Frame;

typedef void (*StateCallback)(World *, void *); // world, and internal state that the callback can use anyway it wants
typedef void (*ConstStateCallback)(const World *, void *);
typedef struct Source Source;
typedef struct Mic Mic;

struct Source {
    StateCallback callback;
    void *state; // Internal state passed to callback to be used for any purpose
    // Some ideas on how state can be used:
    // - As parameters to generic callback functions (frequency to a generic sine wave generator)
    // - As a PRNG state that callback can generate noise out of
    // - Containing a pointer to another Source/Mic to allow for possible middleware shenanigans
    ConstStateCallback state_kill; // To be called at world_kill. Dellocate state and shit here.
};
struct Mic {
    ConstStateCallback callback;
    void *state;
    ConstStateCallback state_kill;
};

struct Frame {
    int width, height;
    float values[];
};
struct World {
    int width, height;
    float time;

    Frame *accelerations;
    Frame *velocities;
    Frame *positions;

    Frame *dampaccelerations; // walls/refraction
    Frame *dampvelocities;    // damping
    Frame *damppositions;     // walls

    int num_sources, num_mics;
    Source *sources;
    Mic *mics;
};

float frame_read(Frame *frame, int x, int y, float def);
void frame_write(Frame *frame, int x, int y, float value);
void world_kill(World *world);
World *world_init(int width, int height);
void world_tick(World *world, float delta);
void world_add_source(World *world, Source source);
void world_add_mic(World *world, Mic mic);


#endif // WAVESIM_SIM_H_
