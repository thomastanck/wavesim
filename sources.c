#include <stdlib.h>
#include <math.h>

#include "mathutils.h"
#include "sim.h"

// SineWaveSource

typedef struct SineWaveSource_state {
    float frequency, amplitude;
    int x, y;
} SineWaveSource_state;
void SineWaveSource_callback(World *world, void *state_ptr) {
    SineWaveSource_state *state = (SineWaveSource_state *)state_ptr;
    float frequency = state->frequency;
    float amplitude = state->amplitude;
    int x = state->x;
    int y = state->y;
    frame_write(world->positions, x, y, amplitude * sin(world->time * TAU * frequency));
    // printf("%f\n", world->time);
    // printf("%d, %d, %f\n", x, y, amplitude * sin(world->time * TAU * frequency));
}
void SineWaveSource_state_kill(const World *world, void *state) {
    free(state);
}
Source SineWaveSource(float frequency, float amplitude, int x, int y) {
    Source source;
    SineWaveSource_state *state = malloc(sizeof(SineWaveSource_state));
    state->frequency = frequency;
    state->amplitude = amplitude;
    state->x = x;
    state->y = y;
    source.callback = SineWaveSource_callback;
    source.state = state;
    source.state_kill = SineWaveSource_state_kill;
    return source;
}
