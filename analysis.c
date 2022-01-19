#include "analysis.h"

#include "tiim.h"
#include "isp.h"
#include "utility.h"

#include <math.h>
#include <stdlib.h>

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

    // Image processing results
    stats->totalCounts = 0.0;
    stats->x1 = 0.0;
    stats->y1 = 0.0;
    stats->agcControlValue = 0.0;
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

            x = k / TII_ROWS;
            y = (TII_ROWS - 1) - (k % TII_ROWS);
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

void onboardProcessing(uint16_t *correctedPixels, bool gotImage, uint16_t minCol, uint16_t maxCol, uint16_t nCols, double *totalCounts, double *x1, double *y1, double *agcControlValue)
{
    // image total
    double total = 0.;
    double x1Total = 0.;
    double y1Total = 0.;
    double columnSum[TII_COLS];
    // column sum values and number of nonzero pixels
    // In one array for easy sorting
    double columnSumsAndNonZeroPixels[TII_COLS*2];
    for (int i = 0; i < TII_COLS; i++)
    {
        columnSum[i] = 0;
        columnSumsAndNonZeroPixels[2*i] = 0;
        columnSumsAndNonZeroPixels[2*i+1] = 0;
    }

    if (!gotImage)
    {
        *totalCounts = 0.0;
        *x1 = 0.0;
        *y1 = 0.0;
        *agcControlValue = 0.0;
    }

    double x, y, value, agcValue;
    for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
    {
        value = (double)correctedPixels[i];
        total += value;
        if (value > 0)
        {
            columnSum[i / TII_ROWS] += value;
            columnSumsAndNonZeroPixels[2*(i / TII_ROWS)] += value;
            columnSumsAndNonZeroPixels[2*(i / TII_ROWS) + 1] ++;
        }
        
        x = 65.0 - (double) (i/TII_ROWS);
        y = 65.0 - (double) (i % TII_ROWS);
        if (x >= minCol && x <= maxCol && y >=1 && y <= 64)
        {
            x1Total += x * value;
            y1Total += y * value;
        }
    }
    // moments
    if (total > 0.0)
    {
        *x1 = x1Total / total;
        *y1 = y1Total / total;
        *totalCounts = total;
    }
    else
    {
        *x1 = 0.0;
        *y1 = 0.0;
        *totalCounts = 0.0;
    }

    // Takes into account that analysis is done only between minCol and maxCol since the columnSumSorted is 0 outsize this anyway
    // Reverse sort so largest is at beginning of array
    if (total > 0)
    {
        qsort(columnSumsAndNonZeroPixels + 2, TII_COLS-1, 2*sizeof(double), &reverseSortDouble);
        *agcControlValue = columnSumsAndNonZeroPixels[2] / columnSumsAndNonZeroPixels[3];
    }
    else
    {
        *agcControlValue = 0.0;
    }

    return;
}

int reverseSortDouble(const void * first, const void *second)
{
    double a = *((double*)first);
    double b = *((double*)second);
    if (a > b)
        return -1;
    else if (a == b)
        return 0;
    else return 1;
}

int histogram(double* values, size_t nValues, double binWidth, double minValue, double maxValue, double **binnedValues, double **binnedCounts, size_t *nBins, int normalization)
{
    int bins = 0;
    *binnedValues = NULL;
    *binnedCounts = NULL;
    if (binWidth <= 0)
        return ANALYSIS_HISTOGRAM_NO_DATA;

    // From min to max inclusive, center of bin first bin is minValue + binWidth/2
    // include in bin at lower bin value and up to and not including upper bin value
    bins = (size_t) ((maxValue - minValue) / binWidth) ;
    *nBins = bins;

    *binnedValues = (double*)malloc(bins * sizeof(double));
    if (*binnedValues == NULL)
        return ANALYSIS_HISTOGRAM_MALLOC;
    *binnedCounts = (double*)malloc(bins * sizeof(double));
    if (*binnedCounts == NULL)
        return ANALYSIS_HISTOGRAM_MALLOC;

    for (size_t i = 0; i < bins; i++)
    {
        (*binnedValues)[i] = minValue + binWidth * i;
        (*binnedCounts)[i] = 0;
    }

    long bin;
    for (size_t i = 0; i < nValues; i++)
    {
        bin = (long) floor((values[i] - minValue) / binWidth); 
        if (bin >= 0 && bin < bins)
        {
            (*binnedCounts)[bin]++;
        }        
    }

    double max = 0.0;
    double total = 0.0;
    switch (normalization)
    {
        case HISTOGRAM_PEAK_EQUALS_ONE:
            for (int i = 0; i < bins; i++)
            {
                if ((*binnedCounts)[i] > max)
                    max = (*binnedCounts)[i];
            }
            if (max > 0)
                for (int i = 0; i < bins; i++)
                    (*binnedCounts)[i] /= max;
            else
                return ANALYSIS_HISTOGRAM_UNNORMALIZED;
            break;
        case HISTOGRAM_AREA_EQUALS_ONE:
            for (int i = 0; i < bins; i++)
                total += (*binnedCounts)[i];
            if (total > 0)
                for (int i = 0; i < bins; i++)
                    (*binnedCounts)[i] /= total;
            else
                return ANALYSIS_HISTOGRAM_UNNORMALIZED;
            break;
        default:
            break;
    }

    return ANALYSIS_OK;
}
