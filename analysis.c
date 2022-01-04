#include "analysis.h"

#include "tiim.h"
#include "isp.h"
#include "utility.h"

#include <math.h>

void initializeImageStats(ImageStats *stats)
{
    stats->cumulativePaCount = 0;
    stats->cumulativeMeaslesCount = 0;
    stats->paCumulativeFrameCount = 0;
    stats->maxPaValue =0;
    for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
    {
        stats->paAngularSpectrumCumulativeFrameCount[b] = 0;
        stats->paAngularSpectrumTotal[b] = 0;
    }

}


void analyzeImage(uint16_t *pixels, bool gotImage, double requestedMaxValue, ImageStats *stats)
{
    double maxValueTmp = 0.0;
    int paCounter = 0;
    int measlesCounter = 0;
    for (int b=0; b < PA_ANGULAR_NUM_BINS; b++)
        stats->paAngularSpectrum[b] = 0;

    int x, y; // pixel coordinates
    double dx, dy, r, phidx, phidy, phi;
    int paBin;
    if (gotImage)
    {
        maxValueTmp = getMaxValue(pixels, requestedMaxValue);
        // Anomaly analysis
        for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++)
        {
            if (pixels[k] == 4095)
                measlesCounter++;

            x = k / 66;
            y = 65 - (k % 66);
            dx = (double) x - OPTICAL_CENTER_X;
            dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
            r = hypot(dx, dy);
            phidx = (double) x - PA_ANGULAR_X0;
            phidy = PA_ANGULAR_X0 - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
            phi = atan2(phidy, phidx) * 180.0 / M_PI;
            if (r >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && pixels[k] != 4095 && pixels[k] > PA_MINIMUM_VALUE)
            {
                paCounter++;
                paBin = getPaBin(phi);
                if (paBin >=0 && paBin < PA_ANGULAR_NUM_BINS)
                {
                    stats->paAngularSpectrum[paBin]++;
                }

            }
        }
    }
    else
        maxValueTmp = 0;

    stats->maxValue = maxValueTmp;
    stats->paCount = paCounter;
    stats->cumulativePaCount += paCounter;
    if (paCounter > 0) stats->paCumulativeFrameCount++;
    for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
    {
        stats->paAngularSpectrumTotal[b] += stats->paAngularSpectrum[b];
        if (stats->paAngularSpectrum[b] > stats->paAngularSpectrumMax[b])
            stats->paAngularSpectrumMax[b] = stats->paAngularSpectrum[b];
        if (stats->paAngularSpectrum[b] > 0)
            stats->paAngularSpectrumCumulativeFrameCount[b]++;
    }

    stats->measlesCount = measlesCounter;
    stats->cumulativeMeaslesCount += measlesCounter;

    return;

}

double getMaxValue(uint16_t *pixels, double requestedMaxValue)
{
    double maxValueTmp = 0.0;
    if (requestedMaxValue < 0.0)
    {
        // autoscale
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

    return maxValueTmp;
}

int getPaBin(double phi)
{
    int paBin = (int)(floor(fmod(phi - PA_ANGULAR_BIN_WIDTH/2.0 + 360.0, 360.0) / PA_ANGULAR_BIN_WIDTH)) - 1;
    return paBin;
}

size_t countImagePairs(ImagePackets *imagePackets, ImagePair *imagePair, double dayStart, double dayEnd)
{
    int status;
    int nImagePairs = 0;
    int imagesRead = 0;

    for (int i = 0; i < imagePackets->numberOfImages-1;)
    {
        status = getAlignedImagePair(imagePackets, i, imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        if (ignoreTime(imagePair->secondsSince1970, dayStart, dayEnd))
            continue;

        nImagePairs++;

    }

    return nImagePairs;

}
