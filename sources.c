#include <stdlib.h>

#include "sources.h"

typedef struct PointSource_state {
    int x, y;
    SourceValue value;
} PointSource_state;
void PointSource_callback(void *state_ptr, World *world) {
    PointSource_state *state = (PointSource_state *)state_ptr;
    frame_write(world->positions, state->x, state->y, state->value.callback(state->value.state, world->time));
}
void PointSource_state_kill(void *state_ptr) {
    PointSource_state *state = (PointSource_state *)state_ptr;
    state->value.state_kill(state->value.state);
    free(state_ptr);
}
Source PointSource(int x, int y, SourceValue value) {
    Source source;
    PointSource_state *state = malloc(sizeof(PointSource_state));
    state->x = x;
    state->y = y;
    state->value = value;
    source.callback = PointSource_callback;
    source.state = state;
    source.state_kill = PointSource_state_kill;
    return source;
}


typedef struct RectSource_state {
    int x1, y1, x2, y2;
    SourceValue value;
} RectSource_state;
void RectSource_callback(void *state_ptr, World *world) {
    RectSource_state *state = (RectSource_state *)state_ptr;
    float value = state->value.callback(state->value.state, world->time);
    for (int x = state->x1; x <= state->x2; x++) {
        for (int y = state->y1; y <= state->y2; y++) {
            frame_write(world->positions, x, y, value);
        }
    }
}
void RectSource_state_kill(void *state_ptr) {
    RectSource_state *state = (RectSource_state *)state_ptr;
    state->value.state_kill(state->value.state);
    free(state_ptr);
}
Source RectSource(int x1, int y1, int x2, int y2, SourceValue value) {
    Source source;
    RectSource_state *state = malloc(sizeof(RectSource_state));
    state->x1 = x1;
    state->y1 = y1;
    state->x2 = x2;
    state->y2 = y2;
    state->value = value;
    source.callback = RectSource_callback;
    source.state = state;
    source.state_kill = RectSource_state_kill;
    return source;
}
