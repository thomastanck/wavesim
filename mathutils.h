#ifndef WAVESIM_MATHUTILS_H_
#define WAVESIM_MATHUTILS_H_

#include <math.h>

#define PI 3.14159265358979
#define TAU (2 * PI)

#define SIGMOID(x) (1 / (1 + exp(-(x))))

#define CUBE(x) (x) * (x) * (x)

// Already defined in opencv
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#endif // WAVESIM_MATHUTILS_H_
