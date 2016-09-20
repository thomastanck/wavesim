#ifndef WAVESIM_SOURCEVALUES_H_
#define WAVESIM_SOURCEVALUES_H_

#include "sim.h"

SourceValue SineWave(float frequency, float amplitude, float phase);
SourceValue SquareWave(float frequency, float amplitude, float phase);
SourceValue TriangleWave(float frequency, float amplitude, float phase);
SourceValue SawtoothWave(float frequency, float amplitude, float phase);
SourceValue Constant(float value);

SourceValue Add(int num_values, SourceValue value, ...);
SourceValue Multiply(int num_values, SourceValue value, ...);
SourceValue Invert(SourceValue value);
SourceValue Shift(float phase, SourceValue value);

#endif // WAVESIM_SOURCEVALUES_H_
