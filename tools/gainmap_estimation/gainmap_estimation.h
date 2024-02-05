/*

    TIIM processing tools: tools/gainmap_estimation/gainmap_estimation.h

    Copyright (C) 2024  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdint.h>

#include "isp.h"
#include "draw.h"

#ifndef _TII_MOMENTS_H
#define _TII_MOMENTS_H

#define GAINMAP_ESTIMATION_VERSION_STRING "1.0"

#define GAINMAP_MAX_VOLTAGES 100
#define GAINMAP_MIN_CAL_IMAGES 5

#define GAINMAP_IMAGE_WIDTH 440
#define GAINMAP_IMAGE_HEIGHT 300
#define GAINMAP_IMAGE_BYTES_PER_PIXEL 1

#define GAINMAP_IMAGE_SCALE 3
#define GAINMAP_IMAGE_RAW_X0 30
#define GAINMAP_IMAGE_RAW_Y0 50
#define GAINMAP_IMAGE_GC_X0 530
#define GAINMAP_IMAGE_GC_Y0 GAINMAP_IMAGE_RAW_Y0
#define GAINMAP_IMAGE_SEPARATION 30

enum GAINMAP_STATUS
{
    GAINMAP_OK = 0,
    GAINMAP_ARGS = 1,
    GAINMAP_NO_IMAGES,
    GAINMAP_CROP_GEOMETRY_FILE,
    GAINMAP_CROP_GEOMETRY_FILE_PARSE,
    GAINMAP_CSV_EXPORT,
    GAINMAP_MEM
};

typedef struct CropMask
{
    double x0;
    double y0;
    double rx;
    double ry;
    // deltas passed as command line options
    double dx0;
    double dy0;
    double drx;
    double dry;
    // previous voltage step used for gain map estimation
    int previousGainMapVoltageStep;

} CropMask;

typedef struct CalData
{
    double calImagery[GAINMAP_MAX_VOLTAGES][NUM_FULL_IMAGE_PIXELS];
    double calVoltages[GAINMAP_MAX_VOLTAGES];
    int numVoltages[GAINMAP_MAX_VOLTAGES];
    CropMask cropMask;
    double max;
    uint16_t minImageMaximum;
    double faceplateVoltage;
    double minBias;
    double maxBias;
    double deltaBias;
    int nBiases;
    
} CalData;

typedef struct State
{
    int nOptions;
    double startTime;
    double endTime;
    bool printImageInfo;
    bool printImage;
    bool printGainMapTable;
    char sensorToPrint;
    int imageIndexToPrint;
    bool exportCsv;
    bool csvUsePreviousStepIndices;
    int csvIndH;
    int csvIndV;
    int pngIndH;
    int pngIndV;
    bool pngAllSteps;
    bool reciprocal;
    bool writePng;
    bool crop;
    bool printPreviousStepIndices;
    bool usePreviousStepIndices;
    char *outDir;
    char *cropGeometryFilename;
    char *startDate;
    char *endDate;
    char satellite;
    char *date1;
    char *date2;
    int nDates;
    double t1;
    double t2;
    
    CalData calDataH;
    CalData calDataV;

} State;

uint16_t imageMax(uint16_t *pixels);
double imageMaxDouble(double *pixels);

void gainMapInfo(const char satellite);

void cropImages(State *state);

int writeImagePng(State *state, int indH, int indV);

void accumulatePixels(State *state, ImagePair *imagePair, char sensor);

void calculateAverages(State *state, char sensor);

double doubleTypeFilter(int k, void *pixelBuf, bool* missing, void* args);

double cropMaskFilter(int k, void *pixelBuf, bool* missing, void* args);

int setCropMasks(State *state);

void drawGainMapPair(Image *imageBuf, ImagePair *imagePair, double maxH, double maxV, int x0, int y0, int scale, int separation, char *labelH, char *labelV, bool showTimestamps, double (*pixelFilter)(int, void*, bool*, void*), void *filterArgsH, void *filterArgsV, char *colorBarFormat);

void drawGainMapColorBar(Image *imageBuf, int xoff, int yoff, int width, int height, double min, double max, char *label, int labelFontSize, int tickFontSize, char *format);

void normalizeGainMaps(CalData *data);

void exportEmptyCsvRow(FILE *file);

int exportCsv(State *state, char sensor, int voltageStepIndex);

#endif // _TII_MOMENTS_H
