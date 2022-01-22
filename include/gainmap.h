#ifndef _GAINMAP_H
#define _GAINMAP_H

#include <stdint.h>

double * getGainMap(int efiUnit, int efiSensor, double time);

void applyGainMap(uint16_t *pixels, double *gainMap, int pixelThreshold, double *maxValue);

#endif // _GAINMAP_H
