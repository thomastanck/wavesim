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
