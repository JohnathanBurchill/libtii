/*

    TIIM processing tools: tools/gainmap_estimation/gainmap_estimation.c

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

#include "gainmap_estimation.h"

#include "parse_args.h"
#include "tii.h"

#include "isp.h"
#include "gainmap.h"
#include "import.h"
#include "analysis.h"
#include "timeseries.h"
#include "png.h"
#include "filters.h"
#include "draw.h"
#include "fonts.h"
#include "colors.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    State state = {0};
    int status = 0;
    
    parseArgs(&state, argc, argv);

    if (state.printGainMapTable)
    {
        gainMapInfo(state.satellite);
        exit(0);
    }

    status = setCropMasks(&state);
    if (status != GAINMAP_OK)
    {
        fprintf(stderr, "Unable to set crop masks. Verify crop file format.\n");
        exit(1);
    }

    if (state.printPreviousStepIndices)
    {
        printf("Previous gain correction map voltage step indices:\n");
        printf("  H: %d\n", state.calDataH.cropMask.previousGainMapVoltageStep);
        printf("  V: %d\n", state.calDataV.cropMask.previousGainMapVoltageStep);
        exit(0);
    }

    char fileDate[32] = {0};
    struct tm *dateInfo = NULL;

    // Data
    ImagePackets imagePackets = {0};

    uint16_t *imagePixels = NULL;
    ImageAuxData *aux = NULL;

    double biasVoltage = 0.0;
    int biasInd = 0;

    time_t theTime = 0;

    size_t numberOfImagePairs = 0;
    ImagePair imagePair;
    ImageAuxData auxH, auxV;
    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS], pixelsV[NUM_FULL_IMAGE_PIXELS];
    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    double dayStart = 0;
    double dayEnd = 0;
    int imagesRead = 0;


    for (int d = 0; d < state.nDates; d++)
    {
        theTime = (time_t)state.t1 + d * 86400;
        dateInfo = gmtime(&theTime);
        sprintf(fileDate, "%c%4d%02d%02d", state.satellite, dateInfo->tm_year + 1900, dateInfo->tm_mon + 1, dateInfo->tm_mday);
        status = importImagery(fileDate, &imagePackets);
        if (status != IMPORT_OK || imagePackets.numberOfImages == 0)
            continue;

        dayStart = (double)theTime;
        dayEnd = dayStart + 86400.0;
        
        numberOfImagePairs = countImagePairs(&imagePackets, &imagePair, dayStart, dayEnd);
        uint16_t pixVal = 0;

        for (size_t i = 0; i < numberOfImagePairs; i++)
        {

            getAlignedImagePair(&imagePackets, 2*i, &imagePair, &imagesRead);

            if (imagePair.gotImageH)
                accumulatePixels(&state, &imagePair, 'H');
            if (imagePair.gotImageV)
                accumulatePixels(&state, &imagePair, 'V');
        }
    }

    // Analyze data
    calculateAverages(&state, 'H');
    calculateAverages(&state, 'V');

    if (state.crop)
        cropImages(&state);

    // Only do this for gain correction maps, not raw images
    if (state.reciprocal)
    {
        normalizeGainMaps(&state.calDataH);
        normalizeGainMaps(&state.calDataV);
    }

    double value = 0.0;
    CalData *data = &state.calDataH;
    if (state.sensorToPrint == 'V')
        data = &state.calDataV;
    if (state.printImageInfo)
    {
        for (int v = 0; v < data->nBiases; v++)
        {
            printf("V[%d] %.2f V (%d images)\n", v, data->calVoltages[v], data->numVoltages[v]);
        }
    }
    else if (state.printImage && !state.exportCsv)
    {
        for (int r = 0; r < TII_ROWS; r++)
        {
            for (int c = 0; c < TII_COLS; c++)
            {
                value = data->calImagery[state.imageIndexToPrint][TII_ROWS * c + r];
                printf("%f ", value);
            }
            printf("\n");
        }
    }

    if (state.exportCsv)
    {
        if (state.csvUsePreviousStepIndices)
        {
            exportCsv(&state, 'H', state.calDataH.cropMask.previousGainMapVoltageStep);
            exportCsv(&state, 'V', state.calDataV.cropMask.previousGainMapVoltageStep);
        }
        else
        {
            exportCsv(&state, 'H', state.csvIndH);
            exportCsv(&state, 'V', state.csvIndV);
        }
    }

    if (state.writePng)
    {
        if (state.pngAllSteps)
        {
            for (int v = 0; v < data->nBiases; v++)
                status = writeImagePng(&state, v, v);
        }
        else if (state.usePreviousStepIndices)
        {
            status = writeImagePng(&state, state.calDataH.cropMask.previousGainMapVoltageStep, state.calDataV.cropMask.previousGainMapVoltageStep);
        }
        else
        {
            status = writeImagePng(&state, state.pngIndH, state.pngIndV);
        }
        
    }

cleanup:
 
    fflush(stdout);

    exit(0);
}

uint16_t imageMax(uint16_t *pixels)
{
    uint16_t max = 0;
    for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
    {
        if (pixels[i] > max)
            max = pixels[i];
    }
    return max;
}

double imageMaxDouble(double *pixels)
{
    double max = 0;
    for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
    {
        if (pixels[i] > max)
            max = pixels[i];
    }
    return max;
}

void gainMapInfo(const char satellite)
{
    int nMaps = 0;
    double *mapTimes = NULL;
    time_t seconds = 0;

    int status = gainMapTimes(satellite, &nMaps, &mapTimes);
    if (status != 0)
    {
        printf("Invalid satellite requested.\n");
        return;
    }    

    struct tm *date;

    if (nMaps > 0)
    {
        printf("Gain map table for Swarm %c\n", satellite);
        printf("MapID\tDate uploaded\n");
        for (int i = 0; i < nMaps; i++)
        {
            seconds = (time_t) floor(mapTimes[i]);
            date = gmtime(&seconds);
            printf("%4d\t%4d%02d%02dT%02d%02d%02d UT\n", i+1, date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
        }
    }

    return;
}

void cropImages(State *state)
{
    for (int v = 0; v < state->calDataH.nBiases; v++)
    {
        for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
        {
            state->calDataH.calImagery[v][i] = cropMaskFilter(i, &state->calDataH.calImagery[v][0], NULL, &state->calDataH);
        }
    }
    for (int v = 0; v < state->calDataV.nBiases; v++)
    {
        for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
        {
            state->calDataV.calImagery[v][i] = cropMaskFilter(i, &state->calDataV.calImagery[v][0], NULL, &state->calDataV);
        }
    }
}


int writeImagePng(State *state, int indH, int indV)
{
    if (indH < 0 || indH >= state->calDataH.nBiases || indV < 0 || indV >= state->calDataV.nBiases)
        return GAINMAP_ARGS;

    // Export PNG
    Image imageBuf;
    if (allocImage(&imageBuf, GAINMAP_IMAGE_WIDTH, GAINMAP_IMAGE_HEIGHT, 1) != DRAW_OK)
    {
        printf("Won't be able to remember the image, sorry.\n");
        return GAINMAP_MEM;
    }
    struct spng_plte colors = getColorTable();

    char filename[FILENAME_MAX];
    double maxH = 0.0;
    double maxV = 0.0;
    ImagePair imagePair = {0};
    ImageAuxData auxH = {0}, auxV = {0};

    int fontSize = 12;
    int x0 = GAINMAP_IMAGE_WIDTH / 2;
    int y0 = 10;
    char text[256] = {0};
    imagePair.gotImageH = true;
    imagePair.gotImageV = true;
    imagePair.auxH = &auxH;
    imagePair.auxV = &auxV;
    uint16_t *imgH = NULL;
    uint16_t *imgV = NULL;

    snprintf(filename, FILENAME_MAX, "%s/Swarm%c_gainmaps_%s_H%02dV%02d.png", state->outDir, state->satellite, state->date2, indH, indV);
    memset(imageBuf.pixels, BACKGROUND_COLOR, imageBuf.numberOfBytes);
    if (state->calDataH.max == -1.0)
        maxH = imageMaxDouble(state->calDataH.calImagery[indH]);
    else
        maxH = state->calDataH.max;
    if (state->calDataV.max == -1.0)
        maxV = imageMaxDouble(state->calDataV.calImagery[indV]);
    else
        maxV = state->calDataV.max;
    imagePair.pixelsH = (uint16_t *)state->calDataH.calImagery[indH];
    imagePair.pixelsV = (uint16_t *)state->calDataV.calImagery[indV];

    if (state->reciprocal)
        drawGainMapPair(&imageBuf, &imagePair, maxH, maxV, GAINMAP_IMAGE_RAW_X0, GAINMAP_IMAGE_RAW_Y0, GAINMAP_IMAGE_SCALE, GAINMAP_IMAGE_SEPARATION, "H", "V", true, &doubleTypeFilter, NULL, NULL, "%.2lf");
    else
        drawImagePair(&imageBuf, &imagePair, maxH, maxV, GAINMAP_IMAGE_RAW_X0, GAINMAP_IMAGE_RAW_Y0, GAINMAP_IMAGE_SCALE, GAINMAP_IMAGE_SEPARATION, "H", "V", true, &doubleTypeFilter, NULL, NULL);

    snprintf(text, 256, "%s Vf=%.2f\n", state->reciprocal ? "Gain correction" : "Average imagery", state->calDataH.faceplateVoltage);
    fontSize = 12;
    annotate(text, fontSize, x0 - (int)((double)strlen(text)/2.*(double)fontSize/12.*8.), y0, &imageBuf);

    snprintf(text, 256, "Step %02d %.2lf V", indH, state->calDataH.calVoltages[indH]);
    fontSize = 12;
    annotate(text, fontSize, GAINMAP_IMAGE_RAW_X0 + ((GAINMAP_IMAGE_SCALE * TII_COLS) / 2) - (int)((double)strlen(text)/2.*(double)fontSize/12.*8.), y0+25, &imageBuf);

    snprintf(text, 256, "Step %02d %.2lf V", indV, state->calDataV.calVoltages[indV]);
    annotate(text, fontSize, GAINMAP_IMAGE_RAW_X0 + (GAINMAP_IMAGE_SCALE * TII_COLS) + GAINMAP_IMAGE_SCALE * GAINMAP_IMAGE_SEPARATION + ((GAINMAP_IMAGE_SCALE * TII_COLS) / 2) - (int)((double)strlen(text)/2.*(double)fontSize/12.*8.), y0+25, &imageBuf);

    writePng(filename, &imageBuf, &colors);

    return GAINMAP_OK;
}

void accumulatePixels(State *state, ImagePair *imagePair, char sensor)
{
    if (state == NULL || imagePair == NULL)
        return;

    ImageAuxData *aux = imagePair->auxH;
    uint16_t *pixels = imagePair->pixelsH;
    CalData *data = &state->calDataH;
    if (sensor == 'V')
    {
        aux = imagePair->auxV;
        pixels = imagePair->pixelsV;
        data = &state->calDataV;
    }

    double biasVoltage = aux->BiasGridVoltageMonitor;
    int biasInd = (int)floor((biasVoltage - data->maxBias)/data->deltaBias);
    int max = imageMax(pixels);
    if (biasInd >= 0 && biasInd < data->nBiases && aux->PhosphorVoltageMonitor > 3500.0 && aux->McpVoltageMonitor < -1000.0 && max < 1800 && max >= data->minImageMaximum)
    {
        data->numVoltages[biasInd]++;
        data->calVoltages[biasInd] += biasVoltage;
        for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
        {
            data->calImagery[biasInd][p] += (double)pixels[p];
        }
        data->faceplateVoltage = aux->FaceplateVoltageMonitor;
    }

    return;
}


void calculateAverages(State *state, char sensor)
{
    CalData *data = &state->calDataH;
    if (sensor == 'V')
        data = &state->calDataV;

    for (int v = 0; v < data->nBiases; v++)
    {
        if (data->numVoltages[v] >= GAINMAP_MIN_CAL_IMAGES)
        {
            data->calVoltages[v] /= (double)data->numVoltages[v];
            for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
            {
                data->calImagery[v][p] /= (double)data->numVoltages[v];
                if (state->reciprocal && data->calImagery[v][p] != 0.0)
                {
                    data->calImagery[v][p] = 1.0 / data->calImagery[v][p];
                }
            }
        }
        else
        {
            data->calVoltages[v] = nan("");
            for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
            {
                data->calImagery[v][p] = nan("");
            }
        }
    }

    return;

}

double doubleTypeFilter(int k, void *pixelBuf, bool* missing, void* args)
{
    if (missing != NULL)
        *missing = false;
    return ((double*)pixelBuf)[k];
}

double cropMaskFilter(int k, void *pixelBuf, bool* missing, void* args)
{
    if (missing != NULL)
        *missing = false;

    if (args == NULL)
        return nan("");

    CalData *data = (CalData*)args;
    CropMask *m = &data->cropMask;
    // X increases from left to right
    // Y increases from bottom to top
    int x = k / TII_ROWS;
    int y = k % TII_ROWS;
    double dx = (double) x - m->x0;
    double dy = (double) y - m->y0;

    double value = 0.0;
    double del = 0.15;
    // Keep pixel if it lies within the crop ellipse
    bool keep = (dx * dx) / ((m->rx - del) * (m->rx - del)) + (dy * dy) / ((m->ry - del) * (m->ry - del)) <= 1.05;
    if (keep)
        value = ((double*)pixelBuf)[k];

    return value;
}


int setCropMasks(State *state)
{
    if (state == NULL || state->cropGeometryFilename == NULL)
        return GAINMAP_ARGS;

    // Default crop ellipse geometries
    switch(state->satellite)
    {
        case 'A':
            state->calDataH.cropMask.x0 += 16.5;
            state->calDataH.cropMask.y0 += 32.0 - 0.5;
            state->calDataH.cropMask.rx += 11.5;
            state->calDataH.cropMask.ry += 9.0;

            state->calDataV.cropMask.x0 += 19.0;
            state->calDataV.cropMask.y0 += 35.5 - 0.5;
            state->calDataV.cropMask.rx += 13.0;
            state->calDataV.cropMask.ry += 9.5;
            break;
        case 'B':
            state->calDataH.cropMask.x0 += 16.0;
            state->calDataH.cropMask.y0 += 34.5 - 0.5;
            state->calDataH.cropMask.rx += 11.5;
            state->calDataH.cropMask.ry += 9.0;

            state->calDataV.cropMask.x0 += 15.5;
            state->calDataV.cropMask.y0 += 36.0 - 0.5;
            state->calDataV.cropMask.rx += 11.0;
            state->calDataV.cropMask.ry += 7.0;
            break;
        case 'C':
            state->calDataH.cropMask.x0 += 17.5;
            state->calDataH.cropMask.y0 += 29.5 - 0.5;
            state->calDataH.cropMask.rx += 11.5;
            state->calDataH.cropMask.ry += 7.5;

            state->calDataV.cropMask.x0 += 19.5;
            state->calDataV.cropMask.y0 += 38.5 - 0.5;
            state->calDataV.cropMask.rx += 11.5;
            state->calDataV.cropMask.ry += 7.5;
            break;
    }

    // Adjust crop ellipses using stored deltas
    FILE *cropFile = fopen(state->cropGeometryFilename, "r");
    if (cropFile == NULL)
        return GAINMAP_CROP_GEOMETRY_FILE;

    int nScanned = 0;
    double dx = 0;
    double dy = 0;
    double drx = 0;
    double dry = 0;
    char satellite = '\0';
    char sensor = '\0';
    int prevStep = 0;
    char lineBuffer[128] = {0};
    CropMask *mask = NULL;

    while (fgets(lineBuffer, 128, cropFile) != NULL)
    {
        if (lineBuffer[0] == '/' || lineBuffer[0] != state->satellite)
            continue;

        nScanned = sscanf(lineBuffer, "%c %c %lf %lf %lf %lf %d", &satellite, &sensor, &dx, &dy, &drx, &dry, &prevStep);
        if (nScanned != 7)
        {
            fclose(cropFile);
            return GAINMAP_CROP_GEOMETRY_FILE_PARSE;
        }
        switch(sensor)
        {
            case 'H':
                mask = &state->calDataH.cropMask;
                break;
            case 'V':
                mask = &state->calDataV.cropMask;
                break;
            default:
                fclose(cropFile);
                return GAINMAP_CROP_GEOMETRY_FILE_PARSE;
        }
        mask->x0 += dx;
        mask->y0 += dy;
        mask->rx += drx;
        mask->ry += dry;
        mask->previousGainMapVoltageStep = prevStep;
    }
    fclose(cropFile);

    // Adjust crop ellipses by amounts set in command line options
    // Defult adjustment is 0
    state->calDataH.cropMask.x0 += state->calDataH.cropMask.dx0; 
    state->calDataH.cropMask.y0 += state->calDataH.cropMask.dy0; 
    state->calDataH.cropMask.rx += state->calDataH.cropMask.drx; 
    state->calDataH.cropMask.ry += state->calDataH.cropMask.dry;

    state->calDataV.cropMask.x0 += state->calDataV.cropMask.dx0; 
    state->calDataV.cropMask.y0 += state->calDataV.cropMask.dy0; 
    state->calDataV.cropMask.rx += state->calDataV.cropMask.drx; 
    state->calDataV.cropMask.ry += state->calDataV.cropMask.dry; 

    return GAINMAP_OK;
}

void drawGainMapPair(Image *imageBuf, ImagePair *imagePair, double maxH, double maxV, int x0, int y0, int scale, int separation, char *labelH, char *labelV, bool showTimestamps, double (*pixelFilter)(int, void*, bool*, void*), void *filterArgsH, void *filterArgsV, char *colorBarFormat)
{

    int delx = scale * separation;

    // H image
    drawImage(imageBuf, imagePair->pixelsH, imagePair->gotImageH, maxH, x0, y0, scale, showTimestamps, imagePair->auxH, pixelFilter, filterArgsH);
    // V image
    drawImage(imageBuf, imagePair->pixelsV, imagePair->gotImageV, maxV, x0 + scale * TII_COLS + delx, y0, scale, showTimestamps, imagePair->auxV, pixelFilter, filterArgsV);

    // color scales
    int scaleHeight = scale * TII_ROWS * 0.7;
    int scaleWidth = (scale * 10) / 4;
    drawGainMapColorBar(imageBuf, x0 + scale * TII_COLS + 10, y0 + scale * TII_ROWS / 2 + scaleHeight / 2, scaleWidth, scaleHeight, 0, maxH, "", 12, 12, colorBarFormat);
    drawGainMapColorBar(imageBuf, x0 + scale * TII_COLS * 2 + delx + 10, y0 + scale * TII_ROWS / 2 + scaleHeight / 2, scaleWidth, scaleHeight, 0, maxV, "", 12, 12, colorBarFormat);

    int fontSize = 9 + 3*((scale-1));
    if (fontSize < 9) fontSize = 9;
    if (fontSize > 24) fontSize = 24;
    int fontWidth = (int)floor(fontSize/9.*6.);

    annotate(labelH, fontSize, (int)floor(x0 + scale * TII_COLS / 2. - fontWidth*strlen(labelH)/2.), y0 + scale*TII_ROWS, imageBuf);
    annotate(labelV, fontSize, (int)floor(x0 + scale * TII_COLS*1.5 + delx - fontWidth*strlen(labelV)/2.), y0 + scale*TII_ROWS, imageBuf);

    return;
}

void drawGainMapColorBar(Image *imageBuf, int xoff, int yoff, int width, int height, double min, double max, char *label, int labelFontSize, int tickFontSize, char *format)
{
    char buf[255];
    int index;

    for (int x = xoff; x < xoff+width; x++)
    {
        for (int y = 0; y < height-1; y++)
        {
            index = imageBuf->width * (yoff - y) + x;
            if (index >= 0 && index < imageBuf->numberOfBytes)
                imageBuf->pixels[index] = (uint8_t)(y / ((double)height) * (double)(MAX_COLOR_VALUE - MIN_COLOR_VALUE) + (double)MIN_COLOR_VALUE);
        }
    }

    annotate(label, labelFontSize, xoff + width / 2, yoff - height - 3 - 16/12 * labelFontSize, imageBuf);
    sprintf(buf, format, min);
    annotate(buf, tickFontSize, xoff + width + 2, yoff - 16/12 * labelFontSize - 2, imageBuf);
    sprintf(buf, format, max);
    annotate(buf, tickFontSize, xoff + width + 2, yoff - height, imageBuf);

    return;
}

void normalizeGainMaps(CalData *data)
{
    double total = 0.0;
    int nCropPixels = 0;
    double val = 0.0;
    double meanMapValue = 0.0;

    for (int v = 0; v < data->nBiases; v++)
    {
        total = 0.0;
        nCropPixels = 0;
        for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
        {
            val = data->calImagery[v][p];
            if (val > 0)
            {
                total += val;
                nCropPixels++;
            }
        }
        if (nCropPixels != 0)
        {
            meanMapValue = total / (double) nCropPixels;
            if (meanMapValue != 0)
            {
                for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
                    data->calImagery[v][p] /= meanMapValue;
            }
            else
            {
                for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
                    data->calImagery[v][p] = nan("");
            }
        }
        else
        {
            for (int p = 0; p < NUM_FULL_IMAGE_PIXELS; p++)
                data->calImagery[v][p] = nan("");
        }
    }
}

void exportEmptyCsvRow(FILE *file)
{
    for (int dc = 0; dc < 45; dc++)
    {
        fprintf(file, "00000000");
        if (dc < 44)
            fprintf(file, ",");
    }
    fprintf(file, "\n");
    return;
}

int exportCsv(State *state, char sensor, int voltageStepIndex)
{
    CalData *data = &state->calDataH;
    if (sensor == 'V')
        data = &state->calDataV;

    float floatValue = 0.0;
    unsigned char *ptr = (unsigned char*)&floatValue;
    uint32_t uint32Val = 0;

    char outFile[FILENAME_MAX];
    char unit[4] = "FM3";
    if (state->satellite == 'B')
        strcpy(unit, "FM2");
    else if (state->satellite == 'C')
        strcpy(unit, "PFM");

    snprintf(outFile, FILENAME_MAX, "%s/gainCorrectionMap_%s_%c_%s.csv", state->outDir, unit, sensor == 'H' ? 'A' : 'B', state->date2);

    FILE *file = fopen(outFile, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Unable to open %s for writing.\n", outFile);
        return GAINMAP_CSV_EXPORT;
    }

    for (int r = 0; r < TII_ROWS; r++)
    {
        for (int c = 0; c < TII_COLS; c++)
        {
            floatValue = (float) data->calImagery[voltageStepIndex][TII_ROWS * c + r];
            if (r == 0 && c == 0)
            {
                // Print two dummy rows of 45 columns
                for (int dr = 0; dr < 2; dr++)
                    exportEmptyCsvRow(file);

            }
            fprintf(file, "%02X%02X%02X%02X", *(ptr + 3), *(ptr+2), *(ptr+1), *(ptr+0));
            if (c != TII_COLS - 1)
                fprintf(file, ",");

            if (c == TII_COLS - 1)
            {
                // Print five dummy cols at the end
                fprintf(file, ",00000000,00000000,00000000,00000000,00000000");
            }
        }
        fprintf(file, "\n");
        if (r == TII_ROWS - 1)
        {
        // Print two dummy rows of 45 columns
        for (int dr = 0; dr < 2; dr++)
            exportEmptyCsvRow(file);
        }
    }

    fclose(file);

    fprintf(stderr, "Exported gainmap to %s\n", outFile);

    return GAINMAP_OK;
}
