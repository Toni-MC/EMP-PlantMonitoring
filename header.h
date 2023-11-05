#include "mbed.h"

#include <cstdio>
#include <cstring>
#include <string>

#include <bitset>
#include <cassert>
#include <cstddef>


// Global mode enum
enum deviceMode{TEST,NORMAL,ADVANCED};
extern deviceMode mode;

extern Thread threadMeasurements;
void MeasurementsDisplay();
