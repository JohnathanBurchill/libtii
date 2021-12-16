#include "draw.h"

#include "tiim.h"
#include "analysis.h"
#include "isp.h"
#include "fonts.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

void drawImage(uint8_t * imageBuf, ImageAuxData *auxH, uint16_t *pixels1, bool gotHImage, ImageStats *statsH, ImageAuxData *auxV, uint16_t *pixels2, bool gotVImage, ImageStats *statsV)
{
    double v;
    int x, y;
    int imageIndex;

    char months[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";


    memset(imageBuf, BACKGROUND_COLOR, IMAGE_BUFFER_SIZE);

    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        if (gotHImage)
        {
            v = floor((double)pixels1[k] / statsH->maxValue * MAX_COLOR_VALUE);
            if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
            if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        }
        else
        {
            v = FOREGROUND_COLOR;
        }
        x = k / 66;
        y = 65 - (k % 66);
        for (int sx = 0; sx < IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(IMAGE_SCALE*(y)+sy+IMAGE_OFFSET_Y)+(IMAGE_SCALE*(x)+sx+IMAGE_OFFSET_X));
                if (imageIndex > IMAGE_BUFFER_SIZE-1) imageIndex = IMAGE_BUFFER_SIZE -1;
                imageBuf[imageIndex] = v;
            }
        }
    }
    for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
    {
        if (gotVImage)
        {
            v = floor((double)pixels2[k] / statsV->maxValue * MAX_COLOR_VALUE);
            if (v > MAX_COLOR_VALUE) v = MAX_COLOR_VALUE;
            if (v < MIN_COLOR_VALUE) v = MIN_COLOR_VALUE;
        }
        else
        {
            v = FOREGROUND_COLOR;
        }
        x = k / 66;
        y = 65 - (k % 66);
        for (int sx = 0; sx < IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(IMAGE_SCALE*(y)+sy+IMAGE_OFFSET_Y)+(IMAGE_SCALE*(x)+sx + IMAGE_SCALE*V_IMAGE_OFFSET_X + IMAGE_OFFSET_X));
                if (imageIndex > IMAGE_BUFFER_SIZE-1) imageIndex = IMAGE_BUFFER_SIZE -1;
                imageBuf[imageIndex] = v;
            }
        }
    }

    // color scales
    int xoff = IMAGE_OFFSET_X + 270;
    int yoff = IMAGE_OFFSET_Y + IMAGE_SCALE * 66 - 37;
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
    annotate("H", 12, xoff, yoff - MAX_COLOR_VALUE / 2 - 20, imageBuf);
    annotate("V", 12, xoff + cbarWidth + cbarSeparation, yoff - MAX_COLOR_VALUE / 2 - 20, imageBuf);
    annotate("0", 9, xoff+cbarWidth+3, yoff-10, imageBuf);
    annotate("0", 9, xoff + cbarWidth + cbarSeparation + cbarWidth+3, yoff-10, imageBuf);
    sprintf(title, "%d", (int)floor(statsH->maxValue));
    annotate(title, 9, xoff+cbarWidth+3, yoff - MAX_COLOR_VALUE/2+3, imageBuf);
    sprintf(title, "%d", (int)floor(statsV->maxValue));
    annotate(title, 9, xoff + cbarWidth + cbarSeparation + cbarWidth+3, yoff - MAX_COLOR_VALUE/2+3, imageBuf);

    // Aux data
    int mo = (auxH->month-1)*3;
    sprintf(title, "Swarm %c %2d %c%c%c %4d %02d:%02d:%02d UT", auxH->satellite, auxH->day, months[mo], months[mo+1], months[mo+2], auxH->year, auxH->hour, auxH->minute, auxH->second);
    annotate(title, 15, IMAGE_WIDTH/2 - strlen(title)/2*10, 5, imageBuf);

    annotate("H", 15, 85, IMAGE_OFFSET_Y + 200, imageBuf);
    annotate("V", 15, 220, IMAGE_OFFSET_Y + 200, imageBuf);

    // Add times in images for montages
    if (gotHImage)
    {
        sprintf(title, "%c %02d:%02d:%02d UT", auxH->sensor, auxH->hour, auxH->minute, auxH->second);
        annotate(title, 9, 30, 40, imageBuf);
    }
    if (gotVImage)
    {
        sprintf(title, "%c %02d:%02d:%02d UT", auxV->sensor, auxV->hour, auxV->minute, auxV->second);
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
    annotate("       PA:", 12, MONITOR_LABEL_OFFSET_X, 50 + 5 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate(" Total PA:", 12, MONITOR_LABEL_OFFSET_X, 50 + 6 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate("  Measles:", 12, MONITOR_LABEL_OFFSET_X, 50 + 7 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    annotate(" Tot. Msl:", 12, MONITOR_LABEL_OFFSET_X, 50 + 8 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);

    if (gotHImage)
    {
        sprintf(title, "  %6.0lf V", auxH->McpVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.0lf V", auxH->PhosphorVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", auxH->BiasGridVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 2 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", auxH->FaceplateVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 3 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf C", auxH->CcdTemperature);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 4 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsH->paCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 5 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsH->cumulativePaCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 6 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsH->measlesCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 7 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsH->cumulativeMeaslesCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 80, 50 + 8 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
    }

    if (gotVImage)
    {            
        sprintf(title, "  %6.0lf V", auxV->McpVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.0lf V", auxV->PhosphorVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", auxV->BiasGridVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 2 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf V", auxV->FaceplateVoltageMonitor);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 3 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6.1lf C", auxV->CcdTemperature);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 4 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsV->paCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 5 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsV->cumulativePaCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 6 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsV->measlesCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 7 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
        sprintf(title, "  %6d", statsV->cumulativeMeaslesCount);
        annotate(title, 12, MONITOR_LABEL_OFFSET_X + 150, 50 + 8 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, imageBuf);
   }

    return;
    
}