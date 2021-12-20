#include "draw.h"

#include "tiim.h"
#include "analysis.h"
#include "isp.h"
#include "fonts.h"
#include "filters.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

void drawFrame(uint8_t * imageBuf, ImagePair *imagePair, ImageStats *statsH, ImageStats *statsV)
{
    double v;
    int x, y;
    double dx, dy, r, r1, phi;
    int paBin;
    int imageIndex;
    int maxPaH = 0;
    int maxPaV = 0;

    char title[255];

    memset(imageBuf, BACKGROUND_COLOR, IMAGE_BUFFER_SIZE);

    int s = RAW_IMAGE_SCALE;
    int x0 = RAW_IMAGE_OFFSET_X;
    int y0 = RAW_IMAGE_OFFSET_Y;

    // Title (satellite and date, time)
    char months[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int mo = (imagePair->auxH->dateTime.month-1)*3;
    IspDateTime *date = &(imagePair->auxH->dateTime);
    if (!imagePair->gotImageH)
        date = &(imagePair->auxV->dateTime);
    mo = (imagePair->auxH->dateTime.month-1)*3;
    sprintf(title, "Swarm %c %2d %c%c%c %4d", imagePair->auxH->satellite, imagePair->auxH->dateTime.day, months[mo], months[mo+1], months[mo+2], imagePair->auxH->dateTime.year);
    annotate(title, 12, IMAGE_WIDTH/2 - strlen(title)/2*8 - 30, 5, imageBuf);

    sprintf(title, "%02d:%02d:%02d UT", imagePair->auxH->dateTime.hour, imagePair->auxH->dateTime.minute, imagePair->auxH->dateTime.second);
    annotate(title, 12, IMAGE_WIDTH/2 - strlen(title)/2*8 + 2, 5 + 20, imageBuf);

    // Raw image
    drawImagePair(imageBuf, imagePair, statsH->maxValue, statsV->maxValue, x0, y0, RAW_IMAGE_SCALE, RAW_IMAGE_SEPARATION_X, "Raw H", "Raw V", true, &identityFilter, NULL, NULL);

    // Gain corrected
    // adjust pixel values first...
    drawImagePair(imageBuf, imagePair, statsH->maxValue, statsV->maxValue, x0 + GAIN_CORRECTED_OFFSET_X, y0, GAIN_CORRECTED_IMAGE_SCALE, RAW_IMAGE_SEPARATION_X, "GC H", "GC V", false, &identityFilter, NULL, NULL);

    // Intensity scaling for PA region imagery
    for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
    {
        if (statsH->paAngularSpectrumCumulativeFrameCount[b] > maxPaH) maxPaH = statsH->paAngularSpectrumCumulativeFrameCount[b];
        if (statsV->paAngularSpectrumCumulativeFrameCount[b] > maxPaV) maxPaV = statsV->paAngularSpectrumCumulativeFrameCount[b];
    }
    if (maxPaH < 100) maxPaH = 100;
    if (maxPaV < 100) maxPaV = 100;


    drawImagePair(imageBuf, imagePair, maxPaH, maxPaV, PA_REGION_IMAGE_OFFSET_X, PA_REGION_IMAGE_OFFSET_Y, PA_REGION_IMAGE_SCALE, 28, "", "", false, paAngularSpectrumFilter, statsH->paAngularSpectrumCumulativeFrameCount, statsV->paAngularSpectrumCumulativeFrameCount);

    annotate("Number of PA frames", 12, PA_REGION_IMAGE_OFFSET_X + PA_REGION_IMAGE_SCALE * TII_COLS + 28 - strlen("Number of PA frame")*8/2, PA_REGION_IMAGE_OFFSET_Y + PA_REGION_IMAGE_SCALE * TII_ROWS, imageBuf);


    // Aux data
    drawMonitors(imageBuf, imagePair, 20, 220);


    // annotate("# frames", 12, pxoff+pcbarWidth + pcbarSeparation/2 - 25, pyoff + 7, imageBuf);
    // annotate("H", 12, pxoff-1, pyoff - MAX_COLOR_VALUE / 3 - 20, imageBuf);
    // annotate("V", 12, pxoff-1 + pcbarWidth + cbarSeparation, pyoff - MAX_COLOR_VALUE / 3 - 20, imageBuf);
    // annotate("0", 9, pxoff+pcbarWidth+3, pyoff-10, imageBuf);
    // annotate("0", 9, pxoff + pcbarWidth + cbarSeparation + pcbarWidth+3, pyoff-10, imageBuf);
    // sprintf(title, "%d", maxPaH);
    // annotate(title, 9, pxoff+pcbarWidth+3, pyoff - MAX_COLOR_VALUE/3 - 2, imageBuf);
    // sprintf(title, "%d", maxPaV);
    // annotate(title, 9, pxoff + pcbarWidth + cbarSeparation + pcbarWidth+3, pyoff - MAX_COLOR_VALUE/3 - 2, imageBuf);

    return;
    
}


void drawImage(uint8_t *imageBuf, uint16_t * pixels, bool gotImage, double maxValue, int xoff, int yoff, int scale, double (*pixelFilter)(int, void*, bool*, void*), void * filterArgs)
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
    drawImage(imageBuf, imagePair->pixelsH, imagePair->gotImageH, maxH, x0, y0, scale, pixelFilter, filterArgsH);
    // V image
    drawImage(imageBuf, imagePair->pixelsV, imagePair->gotImageV, maxV, x0 + scale * TII_COLS + delx, y0, scale, pixelFilter, filterArgsV);

    // color scales
    int scaleHeight = scale * TII_ROWS * 0.7;
    int scaleWidth = (scale * 10) / 4;
    drawColorBar(imageBuf, x0 + scale * TII_COLS + 10, y0 + scale * TII_ROWS / 2 + scaleHeight / 2, scaleWidth, scaleHeight, 0, maxH, "", 12, 12);
    drawColorBar(imageBuf, x0 + scale * TII_COLS * 2 + delx + 10, y0 + scale * TII_ROWS / 2 + scaleHeight / 2, scaleWidth, scaleHeight, 0, maxV, "", 12, 12);

    annotate(labelH, 15, x0 + scale * TII_COLS / 2 - 8*15/6, y0 + scale*TII_ROWS, imageBuf);
    annotate(labelV, 15, x0 + scale * TII_COLS*1.5 + delx - 8*15/6, y0 + scale*TII_ROWS, imageBuf);

    // Add times in images for montages
    if (showTimestamps && imagePair->gotImageH) drawTimestamp(imageBuf, x0, y0, imagePair->auxH, 9);
    if (showTimestamps && imagePair->gotImageV) drawTimestamp(imageBuf, x0 + scale * TII_COLS + delx, y0, imagePair->auxV, 9);

    return;
}

void drawMonitors(uint8_t *imageBuf, ImagePair *imagePair, int x0, int y0)
{

    char title[255];

    ImageAuxData *auxH = imagePair->auxH;
    ImageAuxData *auxV = imagePair->auxV;

    // Monitors
    annotate("Monitors", 15, x0, y0 + 30, imageBuf);
    annotate("H", 15, x0 + 120, y0 + 30, imageBuf);
    annotate("V", 15, x0 + 120 + 70, y0 + 30, imageBuf);


    annotate("      MCP:", 12, x0, 50 + y0, imageBuf);
    annotate("     Phos:", 12, x0, 50 + LINE_SPACING + y0, imageBuf);
    annotate("  ID Bias:", 12, x0, 50 + 2 * LINE_SPACING + y0, imageBuf);
    annotate("Faceplate:", 12, x0, 50 + 3 * LINE_SPACING + y0, imageBuf);
    annotate("CCD temp.:", 12, x0, 50 + 4 * LINE_SPACING + y0, imageBuf);


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
        annotate(title, 12, x0 + 80, 50 + y0, imageBuf);
        sprintf(title, "  %6.0lf V", imagePair->auxH->PhosphorVoltageMonitor);
        annotate(title, 12, x0 + 80, 50 + LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxH->BiasGridVoltageMonitor);
        annotate(title, 12, x0 + 80, 50 + 2 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxH->FaceplateVoltageMonitor);
        annotate(title, 12, x0 + 80, 50 + 3 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf C", imagePair->auxH->CcdTemperature);
        annotate(title, 12, x0 + 80, 50 + 4 * LINE_SPACING + y0, imageBuf);


        // sprintf(title, "  %6d", statsH->paCount);
        // annotate(title, 12, anomXOff + 80, anomYOff, imageBuf);
        // sprintf(title, "  %6d", statsH->cumulativePaCount);
        // annotate(title, 12, anomXOff + 80, anomYOff + 1 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsH->paCumulativeFrameCount);
        // annotate(title, 12, anomXOff + 80, anomYOff + 2 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsH->measlesCount);
        // annotate(title, 12, anomXOff + 80, anomYOff + 3 * LINE_SPACING, imageBuf);
        // sprintf(title, "  %6d", statsH->cumulativeMeaslesCount);
        // annotate(title, 12, anomXOff + 80, anomYOff + 4 * LINE_SPACING, imageBuf);

    }

    if (imagePair->gotImageV)
    {            
        sprintf(title, "  %6.0lf V", imagePair->auxV->McpVoltageMonitor);
        annotate(title, 12, x0 + 150, 50 + y0, imageBuf);
        sprintf(title, "  %6.0lf V", imagePair->auxV->PhosphorVoltageMonitor);
        annotate(title, 12, x0 + 150, 50 + LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxV->BiasGridVoltageMonitor);
        annotate(title, 12, x0 + 150, 50 + 2 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxV->FaceplateVoltageMonitor);
        annotate(title, 12, x0 + 150, 50 + 3 * LINE_SPACING + y0, imageBuf);
        sprintf(title, "  %6.1lf C", imagePair->auxV->CcdTemperature);
        annotate(title, 12, x0 + 150, 50 + 4 * LINE_SPACING + y0, imageBuf);


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
