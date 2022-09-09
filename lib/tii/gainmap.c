/*

    TIIM processing library: lib/tii/gainmap.c

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

#include "gainmap.h"

#include "gainmapdata.h"
#include "isp.h"

#include <stdlib.h>

int gainMapTimes(const char satellite, int *nTimes, double **timeArray)
{
    int n = 0;
    double *timeArr = NULL;
    switch (satellite)
    {
        case 'A':
            n = N_GAIN_MAPS_A;
            timeArr = mapUploadTimesA;
            break;
        case 'B':
            n = N_GAIN_MAPS_B;
            timeArr = mapUploadTimesB;
            break;
        case 'C':
            n = N_GAIN_MAPS_C;
            timeArr = mapUploadTimesC;
            break;

        default:
            return GAIN_MAP_INV_SATELLITE;
    }

    if (nTimes != NULL)
        *nTimes = n;

    if (timeArray != NULL)
        *timeArray = timeArr;

    return GAIN_MAP_OK;

}


double * getGainMap(int efiUnit, int efiSensor, double time)
{
    double * gainmaps;
    double * gainmapUploadTimes;
    int nGainmaps = 0;
    double *map = NULL;
    switch (efiUnit)
    {
        case UNIT_EFI_A:
            nGainmaps = N_GAIN_MAPS_A;
            gainmapUploadTimes = mapUploadTimesA;
            gainmaps = mapsA;
            break;
        case UNIT_EFI_B:
            nGainmaps = N_GAIN_MAPS_B;
            gainmapUploadTimes = mapUploadTimesB;
            gainmaps = mapsB;
            break;
        case UNIT_EFI_C:
            nGainmaps = N_GAIN_MAPS_C;
            gainmapUploadTimes = mapUploadTimesC;
            gainmaps = mapsC;
            break;

        default:
            return NULL;
    }

    for (int i = 0; i < nGainmaps; i++)
    {
        if (time >= gainmapUploadTimes[i])
        {
            // Use this map
            return (gainmaps + (i * 2 * NUM_FULL_IMAGE_PIXELS + efiSensor * NUM_FULL_IMAGE_PIXELS));
        }
    }
    // requested time is before launch
    return NULL;
}


void applyGainMap(uint16_t *pixels, double *gainMap, int pixelThreshold, double *maxValue)
{
    double val;
    double maxValueTmp = 0.0;
    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++)
    {
        val = gainMap[k] * ((double)pixels[k] - pixelThreshold);
        if (val < 0) val = 0;
        pixels[k] = (uint16_t) val;
        if (pixels[k] > maxValueTmp) maxValueTmp = (double) pixels[k];
    }
    if (maxValue != NULL)
        *maxValue = maxValueTmp;
}