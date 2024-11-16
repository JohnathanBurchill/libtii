/*

    TIIM processing library: include/gainmap.h

    Copyright (C) 2022  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
