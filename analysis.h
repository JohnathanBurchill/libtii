#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#include "tiim.h"
#include "isp.h"

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

    double totalCounts;
    double x1;
    double y1;
    double agcControlValue;

} ImageStats;

void initializeImageStats(ImageStats *stats);

void analyzeImage(uint16_t *pixels, bool gotImage, double requestedMaxValue, ImageStats *stats);

double getMaxValue(uint16_t *pixels, double requestedMaxValue);

int getPaBin(double phi);

size_t countImagePairs(ImagePackets *imagePackets, ImagePair *imagePair, double dayStart, double dayEnd);

void onboardProcessing(uint16_t *correctedPixels, bool gotImage, uint16_t minCol, uint16_t maxCol, uint16_t nCols, double *totalCounts, double *x1, double *y1, double *agcControlValue);

int reverseSortDouble(const void * first, const void *second);

#endif // _ANALYSIS_H
