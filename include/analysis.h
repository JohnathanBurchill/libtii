#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#include "settings.h"
#include "isp.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define CLASSIC_WING_ANOMALY_Y2_THRESHOLD 10
#define MEASLES_COUNT_THRESHOLD 10
#define PA_COUNT_THRESHOLD 10
#define ANGELS_WING_THRESHOLD 4550 // average of 91 pixels in 13x7 region exceeds 50 DN

#define MOMENT_MIN_X 10
#define MOMENT_MAX_X 30
#define MOMENT_MIN_Y 20
#define MOMENT_MAX_Y 45
#define Y2_BOX_HALF_WIDTH 7
#define Y2_BOX_HALF_HEIGHT 7
#define BIFURCATION_ANALYSIS_DY 3 // examine above and below y1 by this amount
#define BIFURCATION_ANALYSIS_WIDTH 6
#define BIFURCATION_MINIMUM_PEAK_VALUE 400 // Analyze only images with sufficient signal in the O+ region

enum ANALYSIS_ERR
{
    ANALYSIS_OK = 0,
    ANALYSIS_HISTOGRAM_MALLOC = -1,
    ANALYSIS_HISTOGRAM_NO_DATA = -2,
    ANALYSIS_HISTOGRAM_BIN_WIDTH = -3,
    ANALYSIS_HISTOGRAM_UNNORMALIZED = -4
};

enum HISTOGRAM_NORMALIZATION
{
    HISTOGRAM_UNNORMALIZED = 0,
    HISTOGRAM_PEAK_EQUALS_ONE = 1,
    HISTOGRAM_AREA_EQUALS_ONE = 2
};

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


typedef struct ImageAnomalies {

    bool peripheralAnomaly;
    bool upperAngelsWingAnomaly;
    bool lowerAngelsWingAnomaly;
    bool classicWingAnomaly;
    bool bifurcationAnomaly;
    bool ringAnomaly;
    bool measlesAnomaly;

    // image peak moments (within a small bounding box of the nominal O+ peak location)
    double x1;
    double y1;
    double x2;
    double y2;

    // Angel's wings moments
    double upperAngelsWingX1;
    double upperAngelsWingY1;
    double upperAngelsWingX2;
    double upperAngelsWingY2;
    double lowerAngelsWingX1;
    double lowerAngelsWingY1;
    double lowerAngelsWingX2;
    double lowerAngelsWingY2;

} ImageAnomalies;


void initializeImageStats(ImageStats *stats);

void analyzeImage(uint16_t *pixels, bool gotImage, double requestedMaxValue, ImageStats *stats);

double getMaxValue(uint16_t *pixels, double requestedMaxValue);

int getPaBin(double phi);

size_t countImagePairs(ImagePackets *imagePackets, ImagePair *imagePair, double dayStart, double dayEnd);

void onboardProcessing(uint16_t *correctedPixels, bool gotImage, uint16_t minCol, uint16_t maxCol, uint16_t nCols, double *totalCounts, double *x1, double *y1, double *agcControlValue);

int reverseSortDouble(const void * first, const void *second);

int histogram(double* values, size_t nValues, double binWidth, double minValue, double maxValue, double **binnedValues, double **binnedCounts, size_t *nBins, int normalization);

void applyImagePairGainMaps(ImagePair *imagePair, int pixelThreshold, double *maxH, double *maxV);

void analyzeRawImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies);

void analyzeGainCorrectedImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies);

void initializeAnomalyData(ImageAnomalies *anomalies);



#endif // _ANALYSIS_H
