#ifndef WAVESIM_SOURCES_H_
#define WAVESIM_SOURCES_H_

#include "sim.h"

Source PointSource(int x, int y, SourceValue value);
Source RectSource(int x1, int y1, int x2, int y2, SourceValue value);

#endif // WAVESIM_SOURCES_H_
