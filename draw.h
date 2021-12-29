#ifndef _DRAW_H
#define _DRAW_H

#include "isp.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdint.h>
#include <stdbool.h>

void drawFrame(uint8_t *imageBuf, uint8_t *templateBuf, ImagePair *imagePair, ImageStats *statsH, ImageStats *statsV, LpTiiTimeSeries *timeSeries, int frameCounter);

void drawImage(uint8_t *imageBuf, uint16_t * pixels, bool gotImage, double maxValue, int xoff, int yoff, int scale, double (*pixelFilter)(int, void*, bool*, void *), void* filterArgs);

void drawColorBar(uint8_t *imageBuf, int xoff, int yoff, int width, int height, int min, int max, char *label, int labelFontSize, int tickFontSize);

void drawTimestamp(uint8_t *imageBuf, int x0, int y0, ImageAuxData *aux, int fontSize);

void drawImagePair(uint8_t *imageBuf, ImagePair *imagePair, double maxH, double maxV, int x0, int y0, int scale, int separation, char *labelH, char *labelV, bool showTimestamps, double (*pixelFilter)(int, void*, bool*, void*), void *filterArgsH, void *filterArgsV);

void drawMonitors(uint8_t *imageBuf, ImagePair *imagePair, int x0, int y0);

void drawTemplate(uint8_t * templateBuf, LpTiiTimeSeries *timeSeries);

void drawTimeSeries(uint8_t *imageBuf, double *times, double *values, size_t nValues, int plotX0, int plotY0, int plotWidth, int plotHeight, double t0, double t1, double minValue, double maxValue, const char *xLabel, const char *yLabel, int stride, int colorIndex, const char *minValueStr, const char *maxValueStr, bool log10Scale);

void setBufferColorIndex(uint8_t *imageBuf, int x, int y, int colorIndex);
int rescaleAsInteger(double x, double minX, double maxX, int minScale, int maxScale);

void drawIndicatorLine(uint8_t *imageBuf, int x, int y0, int y1);

void drawFill(uint8_t *imageBuf, int colorIndex);

#endif // _DRAW_H
