#ifndef _DRAW_H
#define _DRAW_H

#include "isp.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdint.h>
#include <stdbool.h>

void drawFrame(uint8_t * imageBuf, ImagePair *imagePair, ImageStats *statsH, ImageStats *statsV, LpTiiTimeSeries *timeSeries, int frameCounter);

void drawImage(uint8_t *imageBuf, uint16_t * pixels, bool gotImage, double maxValue, int xoff, int yoff, int scale, double (*pixelFilter)(int, void*, bool*, void *), void* filterArgs);

void drawColorBar(uint8_t *imageBuf, int xoff, int yoff, int width, int height, int min, int max, char *label, int labelFontSize, int tickFontSize);

void drawTimestamp(uint8_t *imageBuf, int x0, int y0, ImageAuxData *aux, int fontSize);

void drawImagePair(uint8_t *imageBuf, ImagePair *imagePair, double maxH, double maxV, int x0, int y0, int scale, int separation, char *labelH, char *labelV, bool showTimestamps, double (*pixelFilter)(int, void*, bool*, void*), void *filterArgsH, void *filterArgsV);

void drawMonitors(uint8_t *imageBuf, ImagePair *imagePair, int x0, int y0);

#endif // _DRAW_H
