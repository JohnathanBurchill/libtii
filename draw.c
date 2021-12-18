#include "draw.h"

#include "tiim.h"
#include "analysis.h"
#include "isp.h"
#include "fonts.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

void drawImage(uint8_t * imageBuf, ImagePair *imagePair, ImageStats *statsH, ImageStats *statsV)
{
    double v;
    int x, y;
    double dx, dy, r, r1, phi;
    int paBin;
    int imageIndex;
    int maxPaH = 0;
    int maxPaV = 0;

    char months[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";


    memset(imageBuf, BACKGROUND_COLOR, IMAGE_BUFFER_SIZE);

    // Raw H image
    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        if (imagePair->gotImageH)
        {
            if (statsH->maxValue > 0)
                v = floor((double)(imagePair->pixelsH[k]) / statsH->maxValue * MAX_COLOR_VALUE);
            else
                v = MIN_COLOR_VALUE;
            if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
            if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        }
        else
        {
            v = FOREGROUND_COLOR;
        }
        x = k / 66;
        y = 65 - (k % 66);
        for (int sx = 0; sx < RAW_IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < RAW_IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(RAW_IMAGE_SCALE*(y)+sy+IMAGE_OFFSET_Y)+(RAW_IMAGE_SCALE*(x)+sx+IMAGE_OFFSET_X));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    imageBuf[imageIndex] = v;
                }
            }
        }
    }

    // Raw V image
    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        if (imagePair->gotImageV)
        {
            if (statsV->maxValue > 0)
                v = floor((double)(imagePair->pixelsV[k]) / statsV->maxValue * MAX_COLOR_VALUE);
            else
                v = MIN_COLOR_VALUE;
            if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
            if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        }
        else
        {
            v = FOREGROUND_COLOR;
        }
        x = k / 66;
        y = 65 - (k % 66);
        for (int sx = 0; sx < RAW_IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < RAW_IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(RAW_IMAGE_SCALE*(y)+sy+IMAGE_OFFSET_Y)+(RAW_IMAGE_SCALE*(x)+sx + RAW_IMAGE_SCALE*V_IMAGE_OFFSET_X + IMAGE_OFFSET_X));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    imageBuf[imageIndex] = v;
                }
            }
        }
    }

    // color scales
    int xoff = IMAGE_OFFSET_X + 270;
    int yoff = IMAGE_OFFSET_Y + RAW_IMAGE_SCALE * 66 - 30;
    int cbarWidth = 8;
    int cbarSeparation = 30;
    for (int x = xoff; x < xoff+cbarWidth; x++)
    {
        for (int y = MIN_COLOR_VALUE; y <= MAX_COLOR_VALUE; y+=2)
        {
            imageBuf[IMAGE_WIDTH * (yoff - y/2) + x] = y;
            imageBuf[IMAGE_WIDTH * (yoff - y/2) + x + cbarWidth + cbarSeparation] = y;
        }
    }
    char title[40];
    memset(title, 0, 40);

    annotate("DN", 12, xoff+cbarWidth + cbarSeparation/2 - 7, yoff + 7, imageBuf);
    annotate("H", 12, xoff-1, yoff - MAX_COLOR_VALUE / 2 - 20, imageBuf);
    annotate("V", 12, xoff-1 + cbarWidth + cbarSeparation, yoff - MAX_COLOR_VALUE / 2 - 20, imageBuf);
    annotate("0", 9, xoff+cbarWidth+3, yoff-10, imageBuf);
    annotate("0", 9, xoff + cbarWidth + cbarSeparation + cbarWidth+3, yoff-10, imageBuf);
    sprintf(title, "%d", (int)floor(statsH->maxValue));
    annotate(title, 9, xoff+cbarWidth+3, yoff - MAX_COLOR_VALUE/2 - 2, imageBuf);
    sprintf(title, "%d", (int)floor(statsV->maxValue));
    annotate(title, 9, xoff + cbarWidth + cbarSeparation + cbarWidth+3, yoff - MAX_COLOR_VALUE/2 - 2, imageBuf);

    // Aux data
    int mo = (imagePair->auxH->dateTime.month-1)*3;
    if (imagePair->gotImageH)
    {
        mo = (imagePair->auxH->dateTime.month-1)*3;
        sprintf(title, "Swarm %c %2d %c%c%c %4d %02d:%02d:%02d UT", imagePair->auxH->satellite, imagePair->auxH->dateTime.day, months[mo], months[mo+1], months[mo+2], imagePair->auxH->dateTime.year, imagePair->auxH->dateTime.hour, imagePair->auxH->dateTime.minute, imagePair->auxH->dateTime.second);
    }
    else
    {
        mo = (imagePair->auxV->dateTime.month-1)*3;
        sprintf(title, "Swarm %c %2d %c%c%c %4d %02d:%02d:%02d UT", imagePair->auxV->satellite, imagePair->auxV->dateTime.day, months[mo], months[mo+1], months[mo+2], imagePair->auxV->dateTime.year, imagePair->auxV->dateTime.hour, imagePair->auxV->dateTime.minute, imagePair->auxV->dateTime.second);
    }
    annotate(title, 15, IMAGE_WIDTH/2 - strlen(title)/2*10, 5, imageBuf);

    annotate("Raw H", 15, 85 - 8*2.5, IMAGE_OFFSET_Y + 200, imageBuf);
    annotate("Raw V", 15, 220 - 8*2.5, IMAGE_OFFSET_Y + 200, imageBuf);

    // Add times in images for montages
    if (imagePair->gotImageH)
    {
        sprintf(title, "%c %02d:%02d:%02d UT", imagePair->auxH->sensor, imagePair->auxH->dateTime.hour, imagePair->auxH->dateTime.minute, imagePair->auxH->dateTime.second);
        annotate(title, 9, 30, 40, imageBuf);
    }
    if (imagePair->gotImageV)
    {
        sprintf(title, "%c %02d:%02d:%02d UT", imagePair->auxV->sensor, imagePair->auxV->dateTime.hour, imagePair->auxV->dateTime.minute, imagePair->auxV->dateTime.second);
        annotate(title, 9, 165, 40, imageBuf);
    }

    // Monitors
    annotate("H", 15, MONITOR_LABEL_OFFSET_X + 120, 30 + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate("V", 15, MONITOR_LABEL_OFFSET_X + 120 + 70, 30 + MONITOR_LABEL_OFFSET_Y, imageBuf);


    annotate("      MCP:", 12, MONITOR_LABEL_OFFSET_X, 50 + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate("     Phos:", 12, MONITOR_LABEL_OFFSET_X, 50 + LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate("  ID Bias:", 12, MONITOR_LABEL_OFFSET_X, 50 + 2 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate("Faceplate:", 12, MONITOR_LABEL_OFFSET_X, 50 + 3 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate("CCD temp.:", 12, MONITOR_LABEL_OFFSET_X, 50 + 4 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);


    int anomXOff = MONITOR_LABEL_OFFSET_X;
    int anomYOff = 240;
    annotate("       PA:", 12, anomXOff, anomYOff, imageBuf);
    annotate(" Total PA:", 12, anomXOff, anomYOff + 1 * LINE_SPACING, imageBuf);
    annotate("PA frames:", 12, anomXOff, anomYOff + 2 * LINE_SPACING, imageBuf);
    annotate("  Measles:", 12, anomXOff, anomYOff + 3 * LINE_SPACING, imageBuf);
    annotate(" Tot. Msl:", 12, anomXOff, anomYOff + 4 * LINE_SPACING, imageBuf);

    if (imagePair->gotImageH)
    {
        sprintf(title, "  %6.0lf V", imagePair->auxH->McpVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.0lf V", imagePair->auxH->PhosphorVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxH->BiasGridVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 2 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxH->FaceplateVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 3 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf C", imagePair->auxH->CcdTemperature);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 4 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);


        sprintf(title, "  %6d", statsH->paCount);
        annotate(title, 12, anomXOff + 80, anomYOff, imageBuf);
        sprintf(title, "  %6d", statsH->cumulativePaCount);
        annotate(title, 12, anomXOff + 80, anomYOff + 1 * LINE_SPACING, imageBuf);
        sprintf(title, "  %6d", statsH->paCumulativeFrameCount);
        annotate(title, 12, anomXOff + 80, anomYOff + 2 * LINE_SPACING, imageBuf);
        sprintf(title, "  %6d", statsH->measlesCount);
        annotate(title, 12, anomXOff + 80, anomYOff + 3 * LINE_SPACING, imageBuf);
        sprintf(title, "  %6d", statsH->cumulativeMeaslesCount);
        annotate(title, 12, anomXOff + 80, anomYOff + 4 * LINE_SPACING, imageBuf);

    }

    if (imagePair->gotImageV)
    {            
        sprintf(title, "  %6.0lf V", imagePair->auxV->McpVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.0lf V", imagePair->auxV->PhosphorVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxV->BiasGridVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 2 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", imagePair->auxV->FaceplateVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 3 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf C", imagePair->auxV->CcdTemperature);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 4 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);


        sprintf(title, "  %6d", statsV->paCount);
        annotate(title, 12, anomXOff + 150, anomYOff, imageBuf);
        sprintf(title, "  %6d", statsV->cumulativePaCount);
        annotate(title, 12, anomXOff + 150, anomYOff + 1 * LINE_SPACING, imageBuf);
        sprintf(title, "  %6d", statsV->paCumulativeFrameCount);
        annotate(title, 12, anomXOff + 150, anomYOff + 2 * LINE_SPACING, imageBuf);
        sprintf(title, "  %6d", statsV->measlesCount);
        annotate(title, 12, anomXOff + 150, anomYOff + 3 * LINE_SPACING, imageBuf);
        sprintf(title, "  %6d", statsV->cumulativeMeaslesCount);
        annotate(title, 12, anomXOff + 150, anomYOff + 4 * LINE_SPACING, imageBuf);
   }

    for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
    {
        if (statsH->paAngularSpectrumCumulativeFrameCount[b] > maxPaH) maxPaH = statsH->paAngularSpectrumCumulativeFrameCount[b];
        if (statsV->paAngularSpectrumCumulativeFrameCount[b] > maxPaV) maxPaV = statsV->paAngularSpectrumCumulativeFrameCount[b];
    }
    if (maxPaH < 100) maxPaH = 100;
    if (maxPaV < 100) maxPaV = 100;

    // PA region H image
    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        x = k / 66;
        y = 65 - (k % 66);
        dx = (double) x - OPTICAL_CENTER_X;
        dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
        r = hypot(dx, dy);
        r1 = hypot(dx, dy / PA_DY_FACTOR);
        phi = atan2(dy, dx) * 180.0 / M_PI;
        paBin = getPaBin(phi);
        if (maxPaH > 0)
            v = floor(((double)statsH->paAngularSpectrumCumulativeFrameCount[paBin]) / (double) maxPaH * MAX_COLOR_VALUE);
        else
            v = MIN_COLOR_VALUE;
        if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
        if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        for (int sx = 0; sx < PA_REGION_IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < PA_REGION_IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(PA_REGION_IMAGE_SCALE*(y)+sy+PA_REGION_IMAGE_OFFSET_Y)+(PA_REGION_IMAGE_SCALE*(x)+sx+PA_REGION_IMAGE_OFFSET_X));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    if (r1 >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && (paBin >=0 && paBin < PA_ANGULAR_NUM_BINS))
                        imageBuf[imageIndex] = (uint8_t) v;

                }
            }
        }
    }

    // PA region V image
    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        if (imagePair->gotImageV)
        {
            if (statsV->maxValue > 0)
                v = floor((double)(imagePair->pixelsV[k]) / statsV->maxValue * MAX_COLOR_VALUE);
            else v = MIN_COLOR_VALUE;
            if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
            if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        }
        else
        {
            v = FOREGROUND_COLOR;
        }
        x = k / 66;
        y = 65 - (k % 66);
        dx = (double) x - OPTICAL_CENTER_X;
        dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
        r = sqrt(dx * dx + dy * dy);
        r1 = sqrt(dx * dx + dy * dy / (PA_DY_FACTOR * PA_DY_FACTOR));
        phi = atan2(dy, dx) * 180.0 / M_PI;
        paBin = getPaBin(phi);
        if (maxPaV > 0)
            v = floor(((double)statsV->paAngularSpectrumCumulativeFrameCount[paBin]) / (double) maxPaV * MAX_COLOR_VALUE);
        else
            v = MIN_COLOR_VALUE;
        for (int sx = 0; sx < PA_REGION_IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < PA_REGION_IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(PA_REGION_IMAGE_SCALE*(y)+sy+PA_REGION_IMAGE_OFFSET_Y)+(PA_REGION_IMAGE_SCALE*(x)+sx + PA_REGION_IMAGE_SCALE*V_IMAGE_OFFSET_X + PA_REGION_IMAGE_OFFSET_X));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    if (r1 >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && (paBin >=0 && paBin < PA_ANGULAR_NUM_BINS))
                        imageBuf[imageIndex] = (uint8_t) v;
                }
            }
        }
    }
    // PA color bars
    int pxoff = PA_REGION_IMAGE_OFFSET_X + 180;
    int pyoff = PA_REGION_IMAGE_OFFSET_Y + PA_REGION_IMAGE_SCALE * 66 - 30;
    int pcbarWidth = 8;
    int pcbarSeparation = 30;
    for (int x = pxoff; x < pxoff+pcbarWidth; x++)
    {
        for (int y = MIN_COLOR_VALUE; y <= MAX_COLOR_VALUE; y+=2)
        {
            imageBuf[IMAGE_WIDTH * (pyoff - y/3) + x] = y;
            imageBuf[IMAGE_WIDTH * (pyoff - y/3) + x + pcbarWidth + pcbarSeparation] = y;
        }
    }

    annotate("# frames", 12, pxoff+pcbarWidth + pcbarSeparation/2 - 25, pyoff + 7, imageBuf);
    annotate("H", 12, pxoff-1, pyoff - MAX_COLOR_VALUE / 3 - 20, imageBuf);
    annotate("V", 12, pxoff-1 + pcbarWidth + cbarSeparation, pyoff - MAX_COLOR_VALUE / 3 - 20, imageBuf);
    annotate("0", 9, pxoff+pcbarWidth+3, pyoff-10, imageBuf);
    annotate("0", 9, pxoff + pcbarWidth + cbarSeparation + pcbarWidth+3, pyoff-10, imageBuf);
    sprintf(title, "%d", maxPaH);
    annotate(title, 9, pxoff+pcbarWidth+3, pyoff - MAX_COLOR_VALUE/3 - 2, imageBuf);
    sprintf(title, "%d", maxPaV);
    annotate(title, 9, pxoff + pcbarWidth + cbarSeparation + pcbarWidth+3, pyoff - MAX_COLOR_VALUE/3 - 2, imageBuf);

    // PA region annotations
    annotate("PA analysis", 12, 70, PA_REGION_IMAGE_OFFSET_Y-20, imageBuf);
    annotate("H", 12, 65, PA_REGION_IMAGE_OFFSET_Y + 130, imageBuf);
    annotate("V", 12, 155, PA_REGION_IMAGE_OFFSET_Y + 130, imageBuf);



    return;
    
}