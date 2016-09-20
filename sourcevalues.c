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


typedef float Constant_state;
float Constant_callback(void *state_ptr, float time) {
    Constant_state *state = (Constant_state *)state_ptr;
    return *state;
}
void Constant_state_kill(void *state_ptr) {
    free(state_ptr);
}
SourceValue Constant(float value) {
    SourceValue sourcevalue;
    Constant_state *state = malloc(sizeof(Constant_state));
    *state = value;
    sourcevalue.callback = Constant_callback;
    sourcevalue.state = state;
    sourcevalue.state_kill = Constant_state_kill;
    return sourcevalue;
}


typedef struct Add_state {
    int num_values;
    SourceValue *values;
} Add_state;
float Add_callback(void *state_ptr, float time) {
    Add_state *state = (Add_state *)state_ptr;
    float value = 0.0;
    for (int i = 0; i < state->num_values; i++) {
        SourceValue *sourcevalue = &(state->values[i]);
        value += sourcevalue->callback(sourcevalue->state, time);
    }
    return value;
}
void Add_state_kill(void *state_ptr) {
    Add_state *state = (Add_state *)state_ptr;
    for (int i = 0; i < state->num_values; i++) {
        state->values[i].state_kill(state->values[i].state);
    }
    free(state->values);
    free(state_ptr);
}
SourceValue Add(int num_values, SourceValue value, ...) {
    SourceValue *values = malloc(num_values * sizeof(SourceValue));
    for (int i = 0; i < num_values; i++) {
        values[i] = ((SourceValue *)&value)[i];
    }

    SourceValue sourcevalue;
    Add_state *state = malloc(sizeof(Add_state));
    state->num_values = num_values;
    state->values = values;
    sourcevalue.callback = Add_callback;
    sourcevalue.state = state;
    sourcevalue.state_kill = Add_state_kill;
    return sourcevalue;
}


typedef struct Multiply_state {
    int num_values;
    SourceValue *values;
} Multiply_state;
float Multiply_callback(void *state_ptr, float time) {
    Multiply_state *state = (Multiply_state *)state_ptr;
    float value = 1.0;
    for (int i = 0; i < state->num_values; i++) {
        SourceValue *sourcevalue = &(state->values[i]);
        value *= sourcevalue->callback(sourcevalue->state, time);
    }
    return value;
}
void Multiply_state_kill(void *state_ptr) {
    Multiply_state *state = (Multiply_state *)state_ptr;
    for (int i = 0; i < state->num_values; i++) {
        state->values[i].state_kill(state->values[i].state);
    }
    free(state->values);
    free(state_ptr);
}
SourceValue Multiply(int num_values, SourceValue value, ...) {
    SourceValue *values = malloc(num_values * sizeof(SourceValue));
    for (int i = 0; i < num_values; i++) {
        values[i] = ((SourceValue *)&value)[i];
    }

    SourceValue sourcevalue;
    Multiply_state *state = malloc(sizeof(Multiply_state));
    state->num_values = num_values;
    state->values = values;
    sourcevalue.callback = Multiply_callback;
    sourcevalue.state = state;
    sourcevalue.state_kill = Multiply_state_kill;
    return sourcevalue;
}


typedef SourceValue Invert_state;
float Invert_callback(void *state_ptr, float time) {
    Invert_state *value = (Invert_state *)state_ptr;
    return -value->callback(value->state, time);
}
void Invert_state_kill(void *state_ptr) {
    Invert_state *value = (Invert_state *)state_ptr;
    value->state_kill(value->state);
    free(state_ptr);
}
SourceValue Invert(SourceValue value) {
    SourceValue sourcevalue;
    Invert_state *state = malloc(sizeof(Invert_state));
    *state = value;
    sourcevalue.callback = Invert_callback;
    sourcevalue.state = state;
    sourcevalue.state_kill = Invert_state_kill;
    return sourcevalue;
}


typedef struct Shift_state {
    float phase;
    SourceValue value;
} Shift_state;
float Shift_callback(void *state_ptr, float time) {
    Shift_state *state = (Shift_state *)state_ptr;
    return state->value.callback(state->value.state, time + state->phase);
}
void Shift_state_kill(void *state_ptr) {
    Shift_state *state = (Shift_state *)state_ptr;
    state->value.state_kill(state->value.state);
    free(state_ptr);
}
SourceValue Shift(float phase, SourceValue value) {
    SourceValue sourcevalue;
    Shift_state *state = malloc(sizeof(Shift_state));
    state->phase = phase;
    state->value = value;
    sourcevalue.callback = Shift_callback;
    sourcevalue.state = state;
    sourcevalue.state_kill = Shift_state_kill;
    return sourcevalue;
}
