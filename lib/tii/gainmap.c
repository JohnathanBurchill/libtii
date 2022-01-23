#include "gainmap.h"

#include "gainmapdata.h"
#include "isp.h"

#include <stdlib.h>

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