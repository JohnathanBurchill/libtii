#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#include "tiim.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct imageStats
{
    double maxValue;
    double maxPaValue;

    // anomalies
    int paCount;
    int cumulativePaCount;
    int paCumulativeFrameCount;
    int measlesCount;
    int cumulativeMeaslesCount;
    int paAngularSpectrum[PA_ANGULAR_NUM_BINS];
    int paAngularSpectrumMax[PA_ANGULAR_NUM_BINS];
    int paAngularSpectrumTotal[PA_ANGULAR_NUM_BINS];
    int paAngularSpectrumCumulativeFrameCount[PA_ANGULAR_NUM_BINS];

} ImageStats;

void analyzeImage(uint16_t *pixels, bool gotImage, double requestedMaxValue, ImageStats *stats);

double getMaxValue(uint16_t *pixels, double requestedMaxValue);

int getPaBin(double phi);

#endif // _ANALYSIS_H
