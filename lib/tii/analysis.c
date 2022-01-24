#include "analysis.h"

#include "isp.h"
#include "timeseries.h"
#include "utility.h"

#include "gainmap.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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
        stats->paAngularSpectrumMax[b] = 0;
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


void applyImagePairGainMaps(ImagePair *imagePair, int pixelThreshold, double *maxH, double *maxV)
{
    double *gainmap = NULL;

    gainmap = getGainMap(imagePair->auxH->EfiInstrumentId, H_SENSOR, imagePair->auxH->dateTime.secondsSince1970);
    if (gainmap != NULL)
    {
        applyGainMap(imagePair->pixelsH, gainmap, pixelThreshold, maxH);
    }

    gainmap = getGainMap(imagePair->auxV->EfiInstrumentId, V_SENSOR, imagePair->auxV->dateTime.secondsSince1970);
    if (gainmap != NULL)
    {
        applyGainMap(imagePair->pixelsV, gainmap, pixelThreshold, maxV);
    }

}


void analyzeRawImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies)
{
    size_t measlesCounter = 0;
    size_t paCounter = 0;

    size_t angelsWingCounter = 0;

    int x, y; // pixel coordinates
    double dx, dy, r, phidx, phidy, phi;
    double value = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    int imageIndex = 0;

    int awmincol = 29;
    int awmaxcol = 38;
    int awlowerminrow = 14;
    int awlowermaxrow = 26;
    int awupperminrow = 38;
    int awuppermaxrow = 50;

    if (gotImage)
    {
        // Measles, PA
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
                paCounter++;
        }
        if (measlesCounter >= MEASLES_COUNT_THRESHOLD)
            anomalies->measlesAnomaly = true;

        if (paCounter > PA_COUNT_THRESHOLD)
            anomalies->peripheralAnomaly = true;

        // upper angel's wing anomaly
        // Take a vertical rectangle of pixels at upper right
        // If total exceeds threshold, upper angel's wing
        angelsWingCounter = 0;
        x1 = 0;
        y1 = 0;
        for (int c = awmincol; c <= awmaxcol; c++)
            for (int r = awupperminrow; r <= awuppermaxrow; r++)
            {
                imageIndex = c * TII_ROWS + ((TII_ROWS) - r);
                value = (double)pixels[imageIndex];
                angelsWingCounter += (size_t) value;
                x1 += c * value;
                y1 += r * value;
            }
        if (angelsWingCounter > 0)
        {
            x1 /= angelsWingCounter;
            y1 /= angelsWingCounter;
            anomalies->upperAngelsWingX1 = x1;
            anomalies->upperAngelsWingY1 = y1;
            x2 = 0;
            y2 = 0;
            for (int c = awmincol; c <= awmaxcol; c++)
                for (int r = awupperminrow; r <= awuppermaxrow; r++)
                {
                    imageIndex = c * TII_ROWS + ((TII_ROWS) - r);
                    value = (double)pixels[imageIndex];
                    x2 += (c - x1) * (c - x1) * value;
                    y2 += (r - y1) * (r - y1) * value;
                }
            anomalies->upperAngelsWingX2 = x2 / angelsWingCounter;
            anomalies->upperAngelsWingY2 = y2 / angelsWingCounter;
        }
        if (angelsWingCounter > ANGELS_WING_THRESHOLD)
            anomalies->upperAngelsWingAnomaly = true;

        // lower angel's wing anomaly
        // Take a vertical rectangle of pixels at lower right
        // If total exceeds threshold, lower angel's wing
        angelsWingCounter = 0;
        x1 = 0;
        y1 = 0;
        for (int c = awmincol; c <= awmaxcol; c++)
            for (int r = awlowerminrow; r <= awlowermaxrow; r++)
            {
                imageIndex = c * TII_ROWS + ((TII_ROWS) - r);
                value = (double)pixels[imageIndex];
                angelsWingCounter += (size_t) value;
                x1 += c * value;
                y1 += r * value;
            }
        if (angelsWingCounter > 0)
        {
            x1 /= angelsWingCounter;
            y1 /= angelsWingCounter;
            anomalies->lowerAngelsWingX1 = x1;
            anomalies->lowerAngelsWingY1 = y1;
            x2 = 0;
            y2 = 0;
            for (int c = awmincol; c <= awmaxcol; c++)
                for (int r = awlowerminrow; r <= awlowermaxrow; r++)
                {
                    imageIndex = c * TII_ROWS + ((TII_ROWS) - r);
                    value = (double)pixels[imageIndex];
                    x2 += (c - x1) * (c - x1) * value;
                    y2 += (r - y1) * (r - y1) * value;
                }
            anomalies->lowerAngelsWingX2 = x2 / angelsWingCounter;
            anomalies->lowerAngelsWingY2 = y2 / angelsWingCounter;
        }
        if (angelsWingCounter > ANGELS_WING_THRESHOLD)
            anomalies->lowerAngelsWingAnomaly = true;


        // Ring anomaly
        // Not calculated fo now as there is PAs and ring anomaly 
        // have similar effects, if any, on the image's moments.

        // Sum pixels in five regions around an arc at left of image. 
        // If all regions have counts greater than some threshold
        // and central region has higher count than any other
        // and each adjacent region has higher count than the adjacent arc tip regions
        // class ring anomaly 

    }

    return;

}

void analyzeGainCorrectedImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies)
{

    int ix = 0;
    int iy = 0;

    double value = 0;
    double total = 0;
    double x = 0;
    double y = 0;
    double x1 = 0;
    double y1 = 0;
    double x2 = 0;
    double y2 = 0;

    int imageIndex = 0;

    double bifurcationTopTotal = 0;
    double bifurcationBottomTotal = 0;
    double bifurcationMiddleTotal = 0;
    double peakValue = 0;

    if (gotImage)
    {

        // Get first moment
        // Using custom min and max columns to isolate nominal O+ signal
        // Calculate 2nd y moment in box around 1st moment if this is the O+ signal
        for (int i = MOMENT_MIN_X; i <= MOMENT_MAX_X; i++)
            for (int j = MOMENT_MIN_Y; j <= MOMENT_MAX_Y; j++)
            {
                imageIndex = i * TII_ROWS + ((TII_ROWS) - j);
                value = (double)pixels[imageIndex];
                total += value;        
                x1 += i * value;
                y1 += j * value;
            }

        // moments
        if (total > 0.0)
        {
            x1 /= total;
            y1 /= total;

            anomalies->x1 = x1;
            anomalies->y1 = y1;

            ix = (int) floor(x1);
            iy = (int) floor(y1);

            total = 0;
            for (int i = ix - Y2_BOX_HALF_WIDTH; i < ix + Y2_BOX_HALF_HEIGHT; i++)
                for (int j = iy - Y2_BOX_HALF_HEIGHT; j < iy + Y2_BOX_HALF_HEIGHT; j++)
                {
                    imageIndex = i * TII_ROWS + ((TII_ROWS) - j);
                    if (imageIndex < NUM_FULL_IMAGE_PIXELS)
                    {
                        value = (double)pixels[imageIndex];
                        total += value;
                        x2 += (i - x1) * (i - x1) * value;
                        y2 += (j - y1) * (j - y1) * value;
                    }
                }
            if (total > 0)
            {
                x2 /= total;
                y2 /= total;

                anomalies->x2 = x2;
                anomalies->y2 = y2;

                // classic wing anomaly
                if (y2 > CLASSIC_WING_ANOMALY_Y2_THRESHOLD)
                    anomalies->classicWingAnomaly = true;
            }

            // Bifurcation anomaly
            // Get pixel counts above and below y1. 
            // If those values are both larger than pixel count at y1, bifurcation is probable
            for (int i = ix - BIFURCATION_ANALYSIS_WIDTH/2; i <= ix + BIFURCATION_ANALYSIS_WIDTH/2; i++)
            {
                imageIndex = i * TII_ROWS + ((TII_ROWS) - (iy - BIFURCATION_ANALYSIS_DY));
                if (imageIndex < NUM_FULL_IMAGE_PIXELS)
                {
                    value = (double)pixels[imageIndex];
                    bifurcationBottomTotal += value;
                    if (value > peakValue)
                    {
                        peakValue = value;
                    }
                }
                imageIndex = i * TII_ROWS + ((TII_ROWS) + (iy + BIFURCATION_ANALYSIS_DY));
                if (imageIndex < NUM_FULL_IMAGE_PIXELS)
                {
                    value = (double)pixels[imageIndex];
                    bifurcationTopTotal += value;
                    if (value > peakValue)
                    {
                        peakValue = value;
                    }
                }
                imageIndex = i * TII_ROWS + ((TII_ROWS) - iy);
                if (imageIndex < NUM_FULL_IMAGE_PIXELS)
                {
                    value = (double)pixels[imageIndex];
                    bifurcationMiddleTotal += value;
                    if (value > peakValue)
                    {
                        peakValue = value;
                    }
                }
            }
            if (peakValue >= BIFURCATION_MINIMUM_PEAK_VALUE && bifurcationBottomTotal > bifurcationMiddleTotal && bifurcationTopTotal > bifurcationMiddleTotal)
                anomalies->bifurcationAnomaly = true;

        }

    }

    return;

}

void initializeAnomalyData(ImageAnomalies *anomalies)
{
    anomalies->peripheralAnomaly = false;
    anomalies->upperAngelsWingAnomaly = false;
    anomalies->lowerAngelsWingAnomaly = false;
    anomalies->classicWingAnomaly = false;
    anomalies->bifurcationAnomaly = false;
    anomalies->ringAnomaly = false;
    anomalies->measlesAnomaly = false;

    anomalies->x1 = 0.;
    anomalies->y1 = 0.;
    anomalies->x2 = 0.;
    anomalies->y2 = 0.;


    return;
}
