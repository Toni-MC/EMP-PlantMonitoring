#include "mbed.h"

#include <cstdio>
#include <cstring>
#include <string>

#include <bitset>
#include <cassert>
#include <cstddef>

#include <numeric>
#include <algorithm>

// Global mode enum
enum deviceMode{TEST,NORMAL,ADVANCED};
enum Colors{RED,GREEN,BLUE,CYAN,MAGENTA,YELLOW,WHITE,OFF};

extern deviceMode mode;

extern float distanceCM;
extern Thread threadMeasurements;

extern Thread threadDistance;

void MeasurementsDisplay();
// void Distance();
