#include <stdlib.h>

#include "mathutils.h"
#include "sim.h"
#include "display.h"

#define MAX_REACH 1
#define DAMP_SIZE 50

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

// Actual Physics
void update_accelerations1(World *world);
void update_accelerations2(World *world);

void update_accelerations(World *world) {
	// update_accelerations1(world);
	update_accelerations2(world);
}

void update_accelerations1(World *world) {
	// for (int y = 0; y < world->accelerations->height; y++) {
	//	for (int x = 0; x < world->accelerations->width; x++) {

	//		float src = frame_read(world->positions, x, y, 0.0);

	//		float accel = 0;
	//		float norm = 0;
	//		for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
	//			for (int i = - MAX_REACH; i <= MAX_REACH; i++) {

	//				if (i == 0 && j == 0)
	//					continue;
	//				if (out_of_frame(world->positions, x+i, y+j))
	//					continue;

	//				float distance = (i * i + j * j); // (L2 dist) ^ 2

	//				float diff = frame_read(world->positions, x+i, y+j, 0.0) - src;
	//				accel += diff / distance;
	//				norm += distance;
	//			}
	//		}

	//		frame_write(world->accelerations, x, y, accel * norm);
	//	}
	// }
	for (int y = 0; y < world->accelerations->height; y++) {
		for (int x = 0; x < world->accelerations->width; x++) {
			frame_write(world->accelerations, x, y, 0);
		}
	}
	for (int y = 0; y < world->positions->height; y++) {
		for (int x = 0; x < world->positions->width; x++) {
			float src = frame_read(world->positions, x, y, 0.0);

			float norm = 0;
			for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
				for (int i = - MAX_REACH; i <= MAX_REACH; i++) {
					if (i == 0 && j == 0)
						continue;
					if (out_of_frame(world->positions, x+i, y+j))
						continue;
					float distance = (i * i + j * j); // (L2 dist) ^ 2
					norm += 1.0 / distance;
				}
			}
			for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
				for (int i = - MAX_REACH; i <= MAX_REACH; i++) {
					if (i == 0 && j == 0)
						continue;
					if (out_of_frame(world->positions, x+i, y+j))
						continue;
					float distance = (i * i + j * j); // (L2 dist) ^ 2
					float diff = frame_read(world->positions, x+i, y+j, 0.0) - src;
					world->accelerations->values[world->accelerations->width * y + x] += diff / distance / norm;// * world->dampaccelerations->values[world->dampaccelerations->width * (y+j) + (x+i)];
					// int boundary_dist = dist_from_boundary(world->positions, x, y);
					// boundary_dist = 4;
					// if (boundary_dist <= 3) {
					//	world->accelerations->values[world->accelerations->width * y + x] += diff / distance / norm * (boundary_dist + 1) / 4;
					// } else {
					//	world->accelerations->values[world->accelerations->width * y + x] += diff / distance / norm;
					// }
				}
			}
			world->accelerations->values[world->accelerations->width * y + x] *= world->dampaccelerations->values[world->dampaccelerations->width * y + x];
		}
	}
}

void update_accelerations2(World *world) {
	for (int y = 0; y < world->accelerations->height; y++) {
		for (int x = 0; x < world->accelerations->width; x++) {
			frame_write(world->accelerations, x, y, 0);
		}
	}
	for (int y = 0; y < world->positions->height; y++) {
		for (int x = 0; x < world->positions->width; x++) {
			float src = frame_read(world->positions, x, y, 0.0);
			float srcdamp = frame_read(world->dampaccelerations, x, y, 0.0);

			float norm = 0;
			for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
				for (int i = - MAX_REACH; i <= MAX_REACH; i++) {
					if (i == 0 && j == 0)
						continue;
					if (out_of_frame(world->positions, x+i, y+j))
						continue;
					float distance = (i * i + j * j); // (L2 dist) ^ 2
					norm += frame_read(world->dampaccelerations, x+i, y+j, 0.0) / distance;
				}
			}
			for (int j = - MAX_REACH; j <= MAX_REACH; j++) {
				for (int i = - MAX_REACH; i <= MAX_REACH; i++) {
					int destindex = world->positions->width * (y+j) + (x+i);
					if (i == 0 && j == 0)
						continue;
					if (out_of_frame(world->positions, x+i, y+j))
						continue;
					float distance = (i * i + j * j); // (L2 dist) ^ 2
					float diff = src - frame_read(world->positions, x+i, y+j, 0.0);
					world->accelerations->values[destindex] += diff * srcdamp * frame_read(world->dampaccelerations, x+i, y+j, 0.0) / distance / norm;
					// world->accelerations->values[world->accelerations->width * y + x] += diff / distance / norm;// * world->dampaccelerations->values[world->dampaccelerations->width * (y+j) + (x+i)];
				}
			}
			// world->accelerations->values[world->accelerations->width * y + x] *= world->dampaccelerations->values[world->dampaccelerations->width * y + x];
		}
	}
}

void update_velocities(World *world, float delta) {
	for (int y = 0; y < world->velocities->height; y++) {
		for (int x = 0; x < world->velocities->width; x++) {
			int index = y * world->velocities->width + x;
			world->velocities->values[index] += delta * world->accelerations->values[index];
			world->velocities->values[index] *= pow(world->dampvelocities->values[index], delta);
		}
	}
}

void update_positions(World *world, float delta) {
	for (int y = 0; y < world->positions->height; y++) {
		for (int x = 0; x < world->positions->width; x++) {
			int index = y * world->positions->width + x;
			world->positions->values[index] += delta * world->velocities->values[index];
			world->positions->values[index] *= pow(world->damppositions->values[index], delta);
		}
	}
}

// World shit

void world_kill(World *world) {
	if (world->accelerations) frame_kill(world->accelerations);
	if (world->velocities) frame_kill(world->velocities);
	if (world->positions) frame_kill(world->positions);
	for (int i = 0; i < world->num_sources; i++) {
		if (world->sources[i].state_kill != NULL) {
			world->sources[i].state_kill(world->sources[i].state); // Calling function pointer (type StateKillCallback)
		}
	}
	for (int i = 0; i < world->num_mics; i++) {
		if (world->mics[i].state_kill != NULL) {
			world->mics[i].state_kill(world->mics[i].state); // Calling function pointer (type StateKillCallback)
		}
	}
	free(world);
}

World *world_init(int width, int height) {
	World *world = (World *) malloc(sizeof(world));
	if (world == NULL)
		return world;

	world->width = width;
	world->height = height;

	world->time = 0;

	world->accelerations = frame_init(width, height);
	world->velocities = frame_init(width, height);
	world->positions = frame_init(width, height);

	world->dampaccelerations = frame_init(width, height);
	world->dampvelocities = frame_init(width, height);
	world->damppositions = frame_init(width, height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = y * width + x;
			world->dampaccelerations->values[index] = 1;	// walls/refraction
			world->dampvelocities->values[index] = 1;   	// damping
			world->damppositions->values[index] = 1;    	// walls

			// Boundary damping
			int boundary_dist = dist_from_boundary(world->dampvelocities, x, y);
			// boundary_dist = DAMP_SIZE + 1; // Disable damping
			if (boundary_dist <= DAMP_SIZE) {
				world->dampvelocities->values[index] = 1 - CUBE((float)(DAMP_SIZE - boundary_dist) / DAMP_SIZE);
			}

			// Lloyd's mirror
			// if (y == 50) {
			//	world->dampaccelerations->values[index] = 0;
			// }

			// Top fourth is an "optic fibre"
			// if (y > 1 * height / 4 - 5 && y < 1 * height / 4 + 15) {
			//	world->dampaccelerations->values[index] = 0.3;
			// }
			// if (x > 3 * width / 4) {
			//	world->dampaccelerations->values[index] = 1;
			// }

			// if (x > width / 3 && x < 2 * width / 3) {
			//	world->damppositions->values[index] = 0;
			// }
			// if (x > 2 * width / 5 && x < 3 * width / 5) {
			//	world->damppositions->values[index] = 1;
			// }
			// if (y > 1 * height / 4 - 6 && y < 1 * height / 4 + 16) {
			//	world->damppositions->values[index] = 1;
			// }

			if (y > 100 && abs(100-x) < (150-y)) {
				world->dampaccelerations->values[index] = 0.2;
			}
			if (x == 100 && y <= 101) {
				world->dampaccelerations->values[index] = 0.0;
			}
			if (x == 51 || x == 149) {
				world->dampaccelerations->values[index] = 0.0;
			}

			// Wall at right third
			// if (x == 2 * width / 3) {
			//	world->damppositions->values[index] = 0;
			// }
			// Unless at the third points...
			// if (abs(y - height / 3) < 2 || abs(y - 2 * height / 3) < 2) {
			//	world->damppositions->values[index] = 1;
			// }

			// Scatter at bottom left
			// if (x == width / 3 && y == 3 * height / 4) {
			//	world->damppositions->values[index] = 0;
			// }
		}
	}

	world->num_sources = 0;
	world->num_mics = 0;
	world->sources = NULL;
	world->mics = NULL;

	if ((world->accelerations == NULL) ||
		(world->velocities == NULL) ||
		(world->positions == NULL)) {
		world_kill(world);
		return NULL;
	}

	return world;
}

void world_tick(World *world, float delta) {
	update_accelerations(world);
	update_velocities(world, delta);
	update_positions(world, delta);
	for (int i = 0; i < world->num_sources; i++) {
		if (world->sources[i].callback != NULL) {
			world->sources[i].callback(world->sources[i].state, world); // Calling function pointer (type StateCallback)
		}
	}
	for (int i = 0; i < world->num_mics; i++) {
		if (world->mics[i].callback != NULL) {
			world->mics[i].callback(world->mics[i].state, world); // Calling function pointer (type ConstStateCallback)
		}
	}
	world->time += delta;
}

void world_add_source(World *world, Source source) {
	world->num_sources++;
	world->sources = realloc(world->sources, world->num_sources * sizeof(Source));
	world->sources[world->num_sources - 1] = source;
}

void world_add_mic(World *world, Mic mic) {
	world->num_mics++;
	world->mics = realloc(world->mics, world->num_mics * sizeof(Mic));
	world->mics[world->num_mics - 1] = mic;
}
