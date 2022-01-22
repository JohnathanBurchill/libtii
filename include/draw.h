#ifndef _DRAW_H
#define _DRAW_H

#include "tii.h"

#include "isp.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdint.h>
#include <stdbool.h>

void drawFrame(uint8_t * imageBuf, uint8_t *templateBuf, ImagePair *imagePair, LpTiiTimeSeries *timeSeries, ImagePairTimeSeries *imagePairTimeSeries, size_t imagePairIndex, int frameCounter, double dayStart, double dayEnd);

void drawImage(uint8_t *imageBuf, uint16_t * pixels, bool gotImage, double maxValue, int xoff, int yoff, int scale, bool showTimestamps, ImageAuxData *aux, double (*pixelFilter)(int, void*, bool*, void*), void * filterArgs);

void drawColorBar(uint8_t *imageBuf, int xoff, int yoff, int width, int height, int min, int max, char *label, int labelFontSize, int tickFontSize);

void drawTimestamp(uint8_t *imageBuf, int x0, int y0, ImageAuxData *aux, int fontSize);

void drawImagePair(uint8_t *imageBuf, ImagePair *imagePair, double maxH, double maxV, int x0, int y0, int scale, int separation, char *labelH, char *labelV, bool showTimestamps, double (*pixelFilter)(int, void*, bool*, void*), void *filterArgsH, void *filterArgsV);

void drawMonitors(uint8_t *imageBuf, ImagePair *imagePair, int x0, int y0);

void drawTemplate(uint8_t * templateBuf, LpTiiTimeSeries *timeSeries, ImagePairTimeSeries *imagePairTimeSeries, double dayStart, double dayEnd, bool fullDay);

void drawIntTimeSeries(uint8_t *imageBuf, double *times, int *values, size_t nValues, int plotX0, int plotY0, int plotWidth, int plotHeight, double t0, double t1, double minValue, double maxValue, const char *xLabel, const char *yLabel, int stride, int colorIndex, const char *minValueStr, const char *maxValueStr, bool log10Scale, int dotSize, int fontSize, bool axes);

void drawTimeSeries(uint8_t *imageBuf, double *times, double *values, size_t nValues, int plotX0, int plotY0, int plotWidth, int plotHeight, double t0, double t1, double minValue, double maxValue, const char *xLabel, const char *yLabel, int stride, int colorIndex, const char *minValueStr, const char *maxValueStr, bool log10Scale, int dotSize, int fontSize, bool axes);

void setBufferColorIndex(uint8_t *imageBuf, int x, int y, int colorIndex);
int rescaleAsInteger(double x, double minX, double maxX, int minScale, int maxScale);

void drawIndicatorLine(uint8_t *imageBuf, int x, int y0, int y1);
void drawHorizontalLine(uint8_t *imageBuf, int x0, int x1, int y);

void drawFill(uint8_t *imageBuf, int colorIndex);

void drawHistogram(uint8_t *imageBuf, double *values, size_t nValues, double binWidth, double minValue, double maxValue, size_t plotWidth, size_t plotHeight, size_t plotLeft, size_t plotBottom, int normalization, const char *xLabel);

#endif // _DRAW_H
