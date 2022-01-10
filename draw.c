#include "draw.h"

#include "tiim.h"
#include "analysis.h"
#include "isp.h"
#include "gainmap.h"
#include "fonts.h"
#include "filters.h"
#include "utility.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

void drawFrame(uint8_t * imageBuf, uint8_t *templateBuf, ImagePair *imagePair, LpTiiTimeSeries *timeSeries, ImagePairTimeSeries *imagePairTimeSeries, size_t imagePairIndex, int frameCounter, double dayStart, double dayEnd)
{
    double v;
    int x, y;
    double dx, dy, r, r1, phi;
    int paBin;
    int imageIndex;
    int maxPaH = 0;
    int maxPaV = 0;

    double *gainmap;
    // Default values in case there are no config packets
    static size_t lastConfigIndex = 0;
    int threshold = 0;
    int minCol = 33;
    int maxCol = 64;
    int nCols = 32;
    bool agcEnabled = false;
    int agcLower = -1;
    int agcUpper = -1;
    // Get config values if available. All packets must have been sorted.
    for (size_t i = lastConfigIndex; i < timeSeries->nConfig; i++)
    {
        lastConfigIndex = i;
        if (timeSeries->configTime[i] > imagePair->secondsSince1970)
        {
            break;
        }
    }
    if (timeSeries->nConfig > lastConfigIndex)
    {
        threshold = timeSeries->pixelThresholdConfig[lastConfigIndex];
        minCol = timeSeries->tiiMinimumColumnConfig[lastConfigIndex];
        maxCol = timeSeries->tiiMaximumColumnConfig[lastConfigIndex];
        nCols = timeSeries->nColumnsForMomentCalculationsConfig[lastConfigIndex];
        agcEnabled = timeSeries->agcEnabledConfig[lastConfigIndex];
        agcLower = timeSeries->agcLowerThresholdConfig[lastConfigIndex];
        agcUpper = timeSeries->agcUpperThresholdConfig[lastConfigIndex];
    }

    char title[255];

    int s = RAW_IMAGE_SCALE;
    int x0 = RAW_IMAGE_OFFSET_X;
    int y0 = RAW_IMAGE_OFFSET_Y;

    // Title (date, time)
    char months[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int mo = (imagePair->auxH->dateTime.month-1)*3;
    IspDateTime *date = &(imagePair->auxH->dateTime);
    if (!imagePair->gotImageH)
        date = &(imagePair->auxV->dateTime);
    mo = (imagePair->auxH->dateTime.month-1)*3;
    sprintf(title, "Swarm %c %2d %c%c%c %4d", imagePair->auxH->satellite, imagePair->auxH->dateTime.day, months[mo], months[mo+1], months[mo+2], imagePair->auxH->dateTime.year);
    annotate(title, 18, 400, 10, imageBuf);

    sprintf(title, "%02d:%02d:%02d UT", imagePair->auxH->dateTime.hour, imagePair->auxH->dateTime.minute, imagePair->auxH->dateTime.second);
    annotate(title, 15, 460, 38, imageBuf);

    sprintf(title, "Image pair %d", frameCounter+1);
    annotate(title, 12, 100, 5, imageBuf);

    sprintf(title, "Pixel threshold: %d", threshold);
    annotate(title, 9, 20, 490, imageBuf);

    if (agcLower == -1)
        sprintf(title, "AGC lower threshold: unknown");
    else
        sprintf(title, "AGC lower threshold: %d", agcLower);
    annotate(title, 9, 20, 505, imageBuf);
    if (agcUpper == -1)
        sprintf(title, "AGC upper threshold: unknown");
    else
        sprintf(title, "AGC upper threshold: %d", agcUpper);
    annotate(title, 9, 20, 520, imageBuf);

    sprintf(title, "AGC %s", agcEnabled ? "enabled": "disabled");
    annotate(title, 9, 150, 490, imageBuf);

    double maxH = (double)imagePairTimeSeries->maxValueH[imagePairIndex];
    double maxV = (double)imagePairTimeSeries->maxValueV[imagePairIndex];

    // Raw image
    drawImagePair(imageBuf, imagePair, maxH, maxV, x0, y0, RAW_IMAGE_SCALE, RAW_IMAGE_SEPARATION_X, "Raw H", "Raw V", false, &identityFilter, NULL, NULL);

    // Gain corrected
    gainmap = getGainMap(imagePair->auxH->EfiInstrumentId, H_SENSOR, imagePair->auxH->dateTime.secondsSince1970);
    if (gainmap != NULL)
    {
        applyGainMap(imagePair->pixelsH, gainmap, threshold, &maxH);
    }

    gainmap = getGainMap(imagePair->auxV->EfiInstrumentId, V_SENSOR, imagePair->auxV->dateTime.secondsSince1970);
    if (gainmap != NULL)
    {
        applyGainMap(imagePair->pixelsV, gainmap, threshold, &maxV);
    }

    drawImagePair(imageBuf, imagePair, maxH, maxV, GAIN_CORRECTED_OFFSET_X, GAIN_CORRECTED_OFFSET_Y, GAIN_CORRECTED_IMAGE_SCALE, RAW_IMAGE_SEPARATION_X, "GC H", "GC V", false, &identityFilter, NULL, NULL);

    // Intensity scaling for PA region imagery
//    for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
//    {
//        paBin = PA_ANGULAR_NUM_BINS * imagePairIndex + b;
//        if (imagePairTimeSeries->paAngularSpectrumCumulativeFrameCountH[paBin] > maxPaH) maxPaH = imagePairTimeSeries->paAngularSpectrumCumulativeFrameCountH[paBin];
//        if (imagePairTimeSeries->paAngularSpectrumCumulativeFrameCountV[paBin] > maxPaV) maxPaV = imagePairTimeSeries->paAngularSpectrumCumulativeFrameCountV[paBin];
//    }

    maxPaH = 500;
    maxPaV = 500;

    drawImagePair(imageBuf, imagePair, maxPaH, maxPaV, PA_REGION_IMAGE_OFFSET_X, PA_REGION_IMAGE_OFFSET_Y, PA_REGION_IMAGE_SCALE, 28, "", "", false, paAngularSpectrumFilter, &imagePairTimeSeries->paAngularSpectrumCumulativeFrameCountH[PA_ANGULAR_NUM_BINS * imagePairIndex], &imagePairTimeSeries->paAngularSpectrumCumulativeFrameCountV[PA_ANGULAR_NUM_BINS * imagePairIndex]);

    annotate("Number of PA frames", 12, PA_REGION_IMAGE_OFFSET_X + PA_REGION_IMAGE_SCALE * TII_COLS + 28 - strlen("Number of PA frame")*8/2, PA_REGION_IMAGE_OFFSET_Y + PA_REGION_IMAGE_SCALE * TII_ROWS, imageBuf);


    // Aux data
    drawMonitors(imageBuf, imagePair, MONITOR_LABEL_OFFSET_X, MONITOR_LABEL_OFFSET_Y);


    // annotate("# frames", 12, pxoff+pcbarWidth + pcbarSeparation/2 - 25, pyoff + 7, imageBuf);
    // annotate("H", 12, pxoff-1, pyoff - MAX_COLOR_VALUE / 3 - 20, imageBuf);
    // annotate("V", 12, pxoff-1 + pcbarWidth + cbarSeparation, pyoff - MAX_COLOR_VALUE / 3 - 20, imageBuf);
    // annotate("0", 9, pxoff+pcbarWidth+3, pyoff-10, imageBuf);
    // annotate("0", 9, pxoff + pcbarWidth + cbarSeparation + pcbarWidth+3, pyoff-10, imageBuf);
    // sprintf(title, "%d", maxPaH);
    // annotate(title, 9, pxoff+pcbarWidth+3, pyoff - MAX_COLOR_VALUE/3 - 2, imageBuf);
    // sprintf(title, "%d", maxPaV);
    // annotate(title, 9, pxoff + pcbarWidth + cbarSeparation + pcbarWidth+3, pyoff - MAX_COLOR_VALUE/3 - 2, imageBuf);
    int plotWidth = 430;
    int plotHeight0 = 45;
    int plotHeight1 = 30;
    int plotdy = 15;
    
    int plotX0 = 400;
    int plotY0 = 210;
    int plotY1 = plotY0 + plotHeight0 + plotdy;
    int plotY2 = plotY1 + plotHeight0 + plotdy;
    int plotY3 = plotY2 + plotHeight0 + plotdy;
    int plotY4 = plotY3 + plotHeight1 + plotdy;
    int plotY5 = plotY4 + plotHeight1 + plotdy;
    
    x = rescaleAsInteger(imagePair->secondsSince1970, dayStart, dayEnd, plotX0, plotX0 + plotWidth);
    drawIndicatorLine(imageBuf, x, plotY5, plotY0 - plotHeight0);

    return;
    
}


void drawImage(uint8_t *imageBuf, uint16_t * pixels, bool gotImage, double maxValue, int xoff, int yoff, int scale, bool showTimestamps, ImageAuxData *aux, double (*pixelFilter)(int, void*, bool*, void*), void * filterArgs)
{
    double v, val;
    double x, y;
    bool missing = false;
    int imageIndex;

    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        val = (*pixelFilter)(k, pixels, &missing, filterArgs);
        if (maxValue > 0)
            v = floor(val / maxValue * MAX_COLOR_VALUE);
        else
            v = MIN_COLOR_VALUE;
        if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
        if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        if (!gotImage || missing)
            v = BACKGROUND_COLOR;
        x = k / 66;
        y = 65 - (k % 66);
        for (int sx = 0; sx < scale; sx++)
        {
            for (int sy = 0; sy < scale; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(scale*(y)+sy + yoff)+(scale*(x)+sx+xoff));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    imageBuf[imageIndex] = v;
                }
            }
        }
    }

    // Add times in images for montages
    if (gotImage && showTimestamps) drawTimestamp(imageBuf, xoff, yoff, aux, 9);

    return;
}

void drawColorBar(uint8_t *imageBuf, int xoff, int yoff, int width, int height, int min, int max, char *label, int labelFontSize, int tickFontSize)
{
    char buf[255];
    int index;

    for (int x = xoff; x < xoff+width; x++)
    {
        for (int y = 0; y < height-1; y++)
        {
            index = IMAGE_WIDTH * (yoff - y) + x;
            if (index >= 0 && index < IMAGE_BUFFER_SIZE)
                imageBuf[index] = (uint8_t)(y / ((double)height) * (double)(MAX_COLOR_VALUE - MIN_COLOR_VALUE) + (double)MIN_COLOR_VALUE);
        }
    }

    annotate(label, labelFontSize, xoff + width / 2, yoff - height - 3 - 16/12 * labelFontSize, imageBuf);
    sprintf(buf, "%d", min);
    annotate(buf, tickFontSize, xoff + width + 2, yoff - 16/12 * labelFontSize - 2, imageBuf);
    sprintf(buf, "%d", max);
    annotate(buf, tickFontSize, xoff + width + 2, yoff - height, imageBuf);

    return;
}

void drawTimestamp(uint8_t *imageBuf, int x0, int y0, ImageAuxData *aux, int fontSize)
{
    char title[255];
    sprintf(title, "%c %02d:%02d:%02d UT", aux->sensor, aux->dateTime.hour, aux->dateTime.minute, aux->dateTime.second);
    annotate(title, fontSize, x0, y0, imageBuf);

    return;
}


void drawImagePair(uint8_t *imageBuf, ImagePair *imagePair, double maxH, double maxV, int x0, int y0, int scale, int separation, char *labelH, char *labelV, bool showTimestamps, double (*pixelFilter)(int, void*, bool*, void*), void *filterArgsH, void *filterArgsV)
{

    int delx = scale * separation;

    // H image
    drawImage(imageBuf, imagePair->pixelsH, imagePair->gotImageH, maxH, x0, y0, scale, showTimestamps, imagePair->auxV, pixelFilter, filterArgsH);
    // V image
    drawImage(imageBuf, imagePair->pixelsV, imagePair->gotImageV, maxV, x0 + scale * TII_COLS + delx, y0, scale, showTimestamps, imagePair->auxV, pixelFilter, filterArgsV);

    // color scales
    int scaleHeight = scale * TII_ROWS * 0.7;
    int scaleWidth = (scale * 10) / 4;
    drawColorBar(imageBuf, x0 + scale * TII_COLS + 10, y0 + scale * TII_ROWS / 2 + scaleHeight / 2, scaleWidth, scaleHeight, 0, maxH, "", 12, 12);
    drawColorBar(imageBuf, x0 + scale * TII_COLS * 2 + delx + 10, y0 + scale * TII_ROWS / 2 + scaleHeight / 2, scaleWidth, scaleHeight, 0, maxV, "", 12, 12);

    annotate(labelH, 15, x0 + scale * TII_COLS / 2 - 8*15/6, y0 + scale*TII_ROWS, imageBuf);
    annotate(labelV, 15, x0 + scale * TII_COLS*1.5 + delx - 8*15/6, y0 + scale*TII_ROWS, imageBuf);

    return;
}

void drawMonitors(uint8_t *imageBuf, ImagePair *imagePair, int x0, int y0)
{

    char title[255];

    ImageAuxData *auxH = imagePair->auxH;
    ImageAuxData *auxV = imagePair->auxV;

    int font1 = 12;
    int font2 = 9;

    // Monitors
    annotate("Monitors", font1, x0, y0 + 30, imageBuf);
    annotate("H", font1, x0 + 120, y0 + 30, imageBuf);
    annotate("V", font1, x0 + 120 + 70, y0 + 30, imageBuf);


    annotate("      MCP:", font2, x0, 50 + y0, imageBuf);
    annotate("     Phos:", font2, x0, 50 + LINE_SPACING + y0, imageBuf);
    annotate("  ID Bias:", font2, x0, 50 + 2 * LINE_SPACING + y0, imageBuf);
    annotate("Faceplate:", font2, x0, 50 + 3 * LINE_SPACING + y0, imageBuf);
    annotate("CCD temp.:", font2, x0, 50 + 4 * LINE_SPACING + y0, imageBuf);


    // int anomXOff = x0;
    // int anomYOff = 240;
    // annotate("       PA:", 12, anomXOff, anomYOff, imageBuf);
    // annotate(" Total PA:", 12, anomXOff, anomYOff + 1 * LINE_SPACING, imageBuf);
    // annotate("PA frames:", 12, anomXOff, anomYOff + 2 * LINE_SPACING, imageBuf);
    // annotate("  Measles:", 12, anomXOff, anomYOff + 3 * LINE_SPACING, imageBuf);
    // annotate(" Tot. Msl:", 12, anomXOff, anomYOff + 4 * LINE_SPACING, imageBuf);

    if (imagePair->gotImageH)
    {
        sprintf(title, "  %6.0lf V", imagePair->auxH->McpVoltageMonitor);
        annotate(title, font2, x0 + 80, 50 + y0, imageBuf);
        sprintf(title, "  %6.0lf V", imagePair->auxH->PhosphorVoltageMonitor);
        annotate(title, font2, x0 + 80, 50 + LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxH->BiasGridVoltageMonitor);
        annotate(title, font2, x0 + 80, 50 + 2 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxH->FaceplateVoltageMonitor);
        annotate(title, font2, x0 + 80, 50 + 3 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf C", imagePair->auxH->CcdTemperature);
        annotate(title, font2, x0 + 80, 50 + 4 * LINE_SPACING + y0, imageBuf);


        // sprintf(title, "  %6d", statsH->paCount);
        // annotate(title, font2, anomXOff + 80, anomYOff, imageBuf);
        // sprintf(title, "  %6d", statsH->cumulativePaCount);
        // annotate(title, font2, anomXOff + 80, anomYOff + 1 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsH->paCumulativeFrameCount);
        // annotate(title, font2, anomXOff + 80, anomYOff + 2 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsH->measlesCount);
        // annotate(title, font2, anomXOff + 80, anomYOff + 3 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsH->cumulativeMeaslesCount);
        // annotate(title, font2, anomXOff + 80, anomYOff + 4 * LINE_SPACING, imageBuf);

    }

    if (imagePair->gotImageV)
    {            
        sprintf(title, "  %6.0lf V", imagePair->auxV->McpVoltageMonitor);
        annotate(title, font2, x0 + 150, 50 + y0, imageBuf);
        sprintf(title, "  %6.0lf V", imagePair->auxV->PhosphorVoltageMonitor);
        annotate(title, font2, x0 + 150, 50 + LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxV->BiasGridVoltageMonitor);
        annotate(title, font2, x0 + 150, 50 + 2 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxV->FaceplateVoltageMonitor);
        annotate(title, font2, x0 + 150, 50 + 3 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf C", imagePair->auxV->CcdTemperature);
        annotate(title, font2, x0 + 150, 50 + 4 * LINE_SPACING + y0, imageBuf);


        // sprintf(title, "  %6d", statsV->paCount);
        // annotate(title, 12, anomXOff + 150, anomYOff, imageBuf);
        // sprintf(title, "  %6d", statsV->cumulativePaCount);
        // annotate(title, 12, anomXOff + 150, anomYOff + 1 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsV->paCumulativeFrameCount);
        // annotate(title, 12, anomXOff + 150, anomYOff + 2 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsV->measlesCount);
        // annotate(title, 12, anomXOff + 150, anomYOff + 3 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsV->cumulativeMeaslesCount);
        // annotate(title, 12, anomXOff + 150, anomYOff + 4 * LINE_SPACING, imageBuf);
   }

}

void drawTemplate(uint8_t * templateBuf, LpTiiTimeSeries *timeSeries, ImagePairTimeSeries *imagePairTimeSeries, double dayStart, double dayEnd)
{

    // Time series
    int plotWidth = 430;
    int plotHeight0 = 45;
    int plotHeight1 = 30;
    int plotdy = 15;
    
    int plotX0 = 400;
    int plotY0 = 210;
    int plotY1 = plotY0 + plotHeight0 + plotdy;
    int plotY2 = plotY1 + plotHeight0 + plotdy;
    int plotY3 = plotY2 + plotHeight0 + plotdy;
    int plotY4 = plotY3 + plotHeight1 + plotdy;
    int plotY5 = plotY4 + plotHeight1 + plotdy;

    memset(templateBuf, BACKGROUND_COLOR, IMAGE_BUFFER_SIZE);
    // y2
    drawHorizontalLine(templateBuf, plotX0, plotX0 + plotWidth, rescaleAsInteger(10., 0, 20., plotY0, plotY0-plotHeight0));
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->y2H, timeSeries->n2Hz, plotX0, plotY0, plotWidth, plotHeight0, dayStart, dayEnd, 0, 20.0, "", "y2 (pix^2)", 4, MAX_COLOR_VALUE+1, "0", "20", false, 1, 9, true);
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->y2V, timeSeries->n2Hz, plotX0, plotY0, plotWidth, plotHeight0, dayStart, dayEnd, 0, 20.0, "", "", 4, 13, "", "", false, 1, 9, false);

    // Log 10 density
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->ionDensity2, timeSeries->n2Hz, plotX0, plotY1, plotWidth, plotHeight0, dayStart, dayEnd, 4.0, 7.0, "", "log(Ni/cm^-3)", 4, MAX_COLOR_VALUE+1, "4", "7", true, 1, 9, true);

    // PA level
    drawIntTimeSeries(templateBuf, imagePairTimeSeries->time, imagePairTimeSeries->paCountH, imagePairTimeSeries->nImagePairs, plotX0, plotY2, plotWidth, plotHeight0, dayStart, dayEnd, 0, 1000, "", "PA level", 1, MAX_COLOR_VALUE+1, "0", "1000", false, 2, 9, true);
    drawIntTimeSeries(templateBuf, imagePairTimeSeries->time, imagePairTimeSeries->paCountV, imagePairTimeSeries->nImagePairs, plotX0, plotY2, plotWidth, plotHeight0, dayStart, dayEnd, 0, 1000, "", "PA level", 1, 13, "", "", false, 2, 9, false);

    // VMCP
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->mcpVoltageSettingH, timeSeries->n2Hz, plotX0, plotY3, plotWidth, plotHeight0, dayStart, dayEnd, -2400, -1600.0, "", "VMCP (V)", 4, MAX_COLOR_VALUE+1, "-2400", "-1600", false, 1, 9, true);
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->mcpVoltageSettingV, timeSeries->n2Hz, plotX0, plotY3, plotWidth, plotHeight0, dayStart, dayEnd, -2400, -1600.0, "", "", 4, 13, "", "", false, 1, 9, false);

    // VPHOS
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->phosphorVoltageSettingH, timeSeries->n2Hz, plotX0, plotY4, plotWidth, plotHeight1, dayStart, dayEnd, 0, 7000.0, "", "VPhos (V)", 4, MAX_COLOR_VALUE+1, "0", "7000", false, 1, 9, true);
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->phosphorVoltageSettingV, timeSeries->n2Hz, plotX0, plotY4, plotWidth, plotHeight1, dayStart, dayEnd, 0, 7000.0, "", "", 4, 13, "", "", false, 1, 9, false);

    // VBIASGRID
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->biasGridVoltageSettingH, timeSeries->n2Hz, plotX0, plotY5, plotWidth, plotHeight1, dayStart, dayEnd, -100.0, 0.0, "Hours from start of file", "VBias (V)", 4, MAX_COLOR_VALUE+1, "-100", "0", false, 1, 9, true);
    drawTimeSeries(templateBuf, timeSeries->lpTiiTime2Hz, timeSeries->biasGridVoltageSettingV, timeSeries->n2Hz, plotX0, plotY5, plotWidth, plotHeight1, dayStart, dayEnd, -100.0, 0.0, "", "", 4, 13, "", "", false, 1, 9, false);

}



void drawIntTimeSeries(uint8_t *imageBuf, double *times, int *values, size_t nValues, int plotX0, int plotY0, int plotWidth, int plotHeight, double t0, double t1, double minValue, double maxValue, const char *xLabel, const char *yLabel, int stride, int colorIndex, const char *minValueStr, const char *maxValueStr, bool log10Scale, int dotSize, int fontSize, bool axes)
{
    double *dVals = NULL;
    dVals = (double*) malloc(nValues * sizeof(double));
    if (dVals == NULL)
        return;

    for (int i = 0; i < nValues; i++)
    {
        dVals[i] = (double) values[i];
    }
    drawTimeSeries(imageBuf, times, dVals, nValues, plotX0, plotY0, plotWidth, plotHeight, t0, t1, minValue, maxValue, xLabel, yLabel, stride, colorIndex, minValueStr, maxValueStr, log10Scale, dotSize, fontSize, axes);

    free(dVals);
}

void drawTimeSeries(uint8_t *imageBuf, double *times, double *values, size_t nValues, int plotX0, int plotY0, int plotWidth, int plotHeight, double t0, double t1, double minValue, double maxValue, const char *xLabel, const char *yLabel, int stride, int colorIndex, const char *minValueStr, const char *maxValueStr, bool log10Scale, int dotSize, int fontSize, bool axes)
{
    int x0, y0;
    int x, y;

    double timeRange = t1 - t0;
    size_t index;
    double time;
    double tmpVal;
    char label[255];

    if (timeRange > 0 && nValues > 0)
    {
        if (axes)
        {
            // Abscissa
            for (int s = 0; s <= timeRange; s+=60*60)
            {
                sprintf(label, "%d", (int)(s / 3600.));
                annotate(label, fontSize, plotX0 + (int)(s / timeRange * plotWidth)-6, plotY0, imageBuf);
            }
            annotate(xLabel, fontSize, plotX0 + plotWidth/2 - (strlen(xLabel)*(8*fontSize))/24, plotY0+12, imageBuf);
            // Ordinate
            annotate(yLabel, fontSize, plotX0 + plotWidth + 5, plotY0 - plotHeight/2 - 6, imageBuf);
            annotate(minValueStr, fontSize, plotX0 + plotWidth+3, plotY0 - 8, imageBuf);
            annotate(maxValueStr, fontSize, plotX0 + plotWidth+3, plotY0 - plotHeight - 8, imageBuf);
            for (int o = plotY0; o >= plotY0 - plotHeight; o--)
            {
                setBufferColorIndex(imageBuf, plotX0 + plotWidth+1, o, FOREGROUND_COLOR);
            }
        }

        // data
        for (int i = 0; i < nValues; i+=stride)
        {
            if (ignoreTime(times[i], t0, t1))
                continue;

            x0 = rescaleAsInteger(times[i], t0, t1, plotX0, plotX0 + plotWidth);
            tmpVal = values[i];
            if (log10Scale)
            {
                if (tmpVal > 0)
                    tmpVal = log10(tmpVal);
                else
                    tmpVal = -10000.;
            }
            y0 = rescaleAsInteger(tmpVal, minValue, maxValue, plotY0, plotY0 - plotHeight);
            switch (dotSize)
            {
                case 2:
                    setBufferColorIndex(imageBuf, x0, y0, colorIndex);
                    setBufferColorIndex(imageBuf, x0+1, y0, colorIndex);
                    setBufferColorIndex(imageBuf, x0+1, y0+1, colorIndex);
                    setBufferColorIndex(imageBuf, x0, y0+1, colorIndex);
                    break;
                case 3:
                    setBufferColorIndex(imageBuf, x0, y0, colorIndex);
                    setBufferColorIndex(imageBuf, x0, y0-1, colorIndex);
                    setBufferColorIndex(imageBuf, x0, y0+1, colorIndex);
                    setBufferColorIndex(imageBuf, x0+1, y0, colorIndex);
                    setBufferColorIndex(imageBuf, x0+1, y0-1, colorIndex);
                    setBufferColorIndex(imageBuf, x0+1, y0+1, colorIndex);
                    setBufferColorIndex(imageBuf, x0-1, y0, colorIndex);
                    setBufferColorIndex(imageBuf, x0-1, y0-1, colorIndex);
                    setBufferColorIndex(imageBuf, x0-1, y0+1, colorIndex);
                    break;
                default:
                    setBufferColorIndex(imageBuf, x0, y0, colorIndex);
                    break;
            }
        }             
    }
}

void setBufferColorIndex(uint8_t *imageBuf, int x, int y, int colorIndex)
{
    size_t index = IMAGE_WIDTH * y + x;
    if (index >=0 && index <= IMAGE_BUFFER_SIZE-1 && x < IMAGE_WIDTH && x >=0 && y >=0 && y < IMAGE_HEIGHT)
    {
        imageBuf[index] = colorIndex;
    }
}

int rescaleAsInteger(double x, double minX, double maxX, int minScale, int maxScale)
{
    if (minX == maxX) 
    {
        if (x <= minX) return minScale;
        else return maxScale;
    }
    int scaled = (int) floor(minScale + (x - minX) / (maxX - minX) * (double)(maxScale - minScale));
    if (maxScale >= minScale)
    {
        if (scaled < minScale) scaled = minScale;
        if (scaled > maxScale) scaled = maxScale;
    }
    else
    {
        if (scaled > minScale) scaled = minScale;
        if (scaled < maxScale) scaled = maxScale;
    }

    return scaled;
}

void drawIndicatorLine(uint8_t *imageBuf, int x, int y0, int y1)
{
    for (int y = y0; y > y1; y--)
    {
        if (y/5 % 2 == 0)
        {
            setBufferColorIndex(imageBuf, x, y, MAX_COLOR_VALUE + 3);
            setBufferColorIndex(imageBuf, x + 1, y, MAX_COLOR_VALUE + 3);
        }
    }
}

void drawHorizontalLine(uint8_t *imageBuf, int x0, int x1, int y)
{
    for (int x = x0; x < x1; x++)
    {
        if (x/5 % 2 == 0)
        {
            setBufferColorIndex(imageBuf, x, y, MAX_COLOR_VALUE + 3);
            setBufferColorIndex(imageBuf, x, y+1, MAX_COLOR_VALUE + 3);
        }
    }
}

void drawFill(uint8_t *imageBuf, int colorIndex)
{
    memset(imageBuf, colorIndex, IMAGE_BUFFER_SIZE);
}
