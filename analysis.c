#include "analysis.h"

#include "tiim.h"
#include "isp.h"

#include <math.h>


void analyzeImage(uint16_t *pixels, bool gotImage, double requestedMaxValue, ImageStats *stats)
{
    double maxValueTmp = -1.0;
    int paCounter = 0;
    int measlesCounter = 0;

    int x, y; // pixel coordinates
    double dx, dy, r;
    if (gotImage)
    {
        if (requestedMaxValue < 0.0)
        {
            for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++)
            {
                if ((double)pixels[k] > maxValueTmp && pixels[k] < MAX_PIXEL_VALUE_FOR_MAX_CALCULATION) // Ignore measles pixels in max calculation
                    maxValueTmp = pixels[k];
            }
        }
        else
        {
            maxValueTmp = requestedMaxValue;
        }
        // Anomaly analysis
        for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++)
        {
            if (pixels[k] == 4095)
                measlesCounter++;

            x = k / 66;
            y = 65 - (k % 66);
            dx = (double) x - OPTICAL_CENTER_X;
            dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
            r = sqrt(dx * dx + dy * dy);
            if (r >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && pixels[k] != 4095 && pixels[k] > PA_MINIMUM_VALUE)
                paCounter++;
        }
    }
    else
        maxValueTmp = 1;

    stats->maxValue = maxValueTmp;
    stats->paCount = paCounter;
    stats->cumulativePaCount += paCounter;
    stats->measlesCount = measlesCounter;
    stats->cumulativeMeaslesCount += measlesCounter;

    return;

}