// #include "mbed.h"
#include <cstdio>
#include <cstring>
#include <string>

#include "MMA8451Q.h"


/*
const uint32_t FLAG_LOW  = (1UL << 1);
const uint32_t FLAG_MID  = (1UL << 2);
const uint32_t FLAG_HIGH = (1UL << 3);
*/
const uint32_t FLAG_ON = (1UL << 4);

extern int LightLevel;
extern EventFlags Flags;
extern string mode;
extern Thread threadSerial;
void BrightnessSerial();
