#ifndef _GAINMAP_H
#define _GAINMAP_H

#include <stdint.h>

enum GAIN_MAP_STATUS
{
    GAIN_MAP_OK = 0,
    GAIN_MAP_INV_SATELLITE = 1,
    GAIN_MAP_INV_UNIT = 2,
    GAIN_MAP_INV_SENSOR = 3
};

int gainMapTimes(const char satellite, int *nTimes, double **timeArray);

double * getGainMap(int efiUnit, int efiSensor, double time);

void applyGainMap(uint16_t *pixels, double *gainMap, int pixelThreshold, double *maxValue);

#endif // _GAINMAP_H
