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
    double dx, dy, r, r1;
    int imageIndex;

    char months[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";


    memset(imageBuf, BACKGROUND_COLOR, IMAGE_BUFFER_SIZE);

    // Raw H image
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
    int mo = (auxH->month-1)*3;
    sprintf(title, "Swarm %c %2d %c%c%c %4d %02d:%02d:%02d UT", auxH->satellite, auxH->day, months[mo], months[mo+1], months[mo+2], auxH->year, auxH->hour, auxH->minute, auxH->second);
    annotate(title, 15, IMAGE_WIDTH/2 - strlen(title)/2*10, 5, imageBuf);

    annotate("Raw H", 15, 85 - 8*2.5, IMAGE_OFFSET_Y + 200, imageBuf);
    annotate("Raw V", 15, 220 - 8*2.5, IMAGE_OFFSET_Y + 200, imageBuf);

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

    // PA region H image
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
        dx = (double) x - OPTICAL_CENTER_X;
        dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
        r = sqrt(dx * dx + dy * dy);
        r1 = sqrt(dx * dx + dy * dy / (PA_DY_FACTOR * PA_DY_FACTOR));
        for (int sx = 0; sx < PA_REGION_IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < PA_REGION_IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(PA_REGION_IMAGE_SCALE*(y)+sy+PA_REGION_IMAGE_OFFSET_Y)+(PA_REGION_IMAGE_SCALE*(x)+sx+PA_REGION_IMAGE_OFFSET_X));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    if (r1 >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && gotHImage)
                        imageBuf[imageIndex] = v;
                    else if (gotHImage)
                        imageBuf[imageIndex] = BACKGROUND_COLOR;
                    else 
                        imageBuf[imageIndex] = FOREGROUND_COLOR;

                }
            }
        }
    }

    // PA region V image
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
        dx = (double) x - OPTICAL_CENTER_X;
        dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
        r = sqrt(dx * dx + dy * dy);
        r1 = sqrt(dx * dx + dy * dy / (PA_DY_FACTOR * PA_DY_FACTOR));
        for (int sx = 0; sx < PA_REGION_IMAGE_SCALE; sx++)
        {
            for (int sy = 0; sy < PA_REGION_IMAGE_SCALE; sy++)
            {
                imageIndex = (IMAGE_WIDTH*(PA_REGION_IMAGE_SCALE*(y)+sy+PA_REGION_IMAGE_OFFSET_Y)+(PA_REGION_IMAGE_SCALE*(x)+sx + PA_REGION_IMAGE_SCALE*V_IMAGE_OFFSET_X + PA_REGION_IMAGE_OFFSET_X));
                if (imageIndex < IMAGE_BUFFER_SIZE)
                {
                    if (r1 >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && gotHImage)
                        imageBuf[imageIndex] = v;
                    else if (gotHImage)
                        imageBuf[imageIndex] = BACKGROUND_COLOR;
                    else 
                        imageBuf[imageIndex] = FOREGROUND_COLOR;

                }
            }
        }
    }

    // PA region annotations
    annotate("PA analysis", 12, 70, PA_REGION_IMAGE_OFFSET_Y-20, imageBuf);
    annotate("H", 12, 65, PA_REGION_IMAGE_OFFSET_Y + 130, imageBuf);
    annotate("V", 12, 155, PA_REGION_IMAGE_OFFSET_Y + 130, imageBuf);



    return;
    
}