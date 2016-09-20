#include <stdlib.h>
#include <math.h>

#include "sourcevalues.h"
#include "mathutils.h"
#include "sim.h"

// SineWave

typedef struct SineWave_state {
    float frequency, amplitude, phase;
} SineWave_state;
float SineWave_callback(void *state_ptr, float time) {
    SineWave_state *state = (SineWave_state *)state_ptr;
    return state->amplitude * sin(time * TAU * state->frequency + state->phase);
}
void SineWave_state_kill(void *state_ptr) {
    free(state_ptr);
}
SourceValue SineWave(float frequency, float amplitude, float phase) {
    SourceValue sourcevalue;
    SineWave_state *state = malloc(sizeof(SineWave_state));
    state->frequency = frequency;
    state->amplitude = amplitude;
    state->phase = phase;
    sourcevalue.callback = SineWave_callback;
    sourcevalue.state = state;
    sourcevalue.state_kill = SineWave_state_kill;
    return sourcevalue;
}
