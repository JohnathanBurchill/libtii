/*

    TIIM processing tools: tools/tii_look/tii_look.c

    Copyright (C) 2022  Johnathan K Burchill

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

#include "tii_look.h"

#include "tii.h"
#include "tiigraphics.h"
#include "draw.h"
#include "colors.h"
#include "filters.h"
#include "png.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

void usage(const char * name);

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        usage(argv[0]);
        exit(1);
    }

    int status = 0;

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);

    // Daily image statistics only
    if (sourceLen != 9)
    {
        usage(argv[0]);
	    exit(1);
    }
    double requestedMax = atof(argv[2]);
    long imagePairNumber = atol(argv[3]);
    if (imagePairNumber < 1)
    {
        usage(argv[0]);
        exit(1);
    }
    char *outputDir = argv[4];

    // Data
    ImagePackets imagePackets;

    status = importImagery(satDate, &imagePackets);
    if (status)
    {
        fprintf(stderr, "Could not import image data.\n");
        goto cleanup;
    }
    if (imagePackets.numberOfImages == 0)
    {
        fprintf(stderr, "No images found for satellite %c on %s\n", satDate[0], satDate+1);
        goto cleanup;
    }
    
    // If there are no config packets we bail, since we need to be able to apply the gain correction maps
    // and calculate onboard moments.
    SciencePackets sciencePackets;
    LpTiiTimeSeries timeSeries;
    initLpTiiTimeSeries(&timeSeries);
    importScience(satDate, &sciencePackets);
    getLpTiiTimeSeries(satDate[0], &sciencePackets, &timeSeries);

    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS], pixelsV[NUM_FULL_IMAGE_PIXELS];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImagePair imagePair;
    ImageAuxData auxH, auxV;
    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    getFirstImagePair(&imagePackets, &imagePair);
    double dayStart = imagePair.secondsSince1970;
    char satellite = getSatellite(&imagePair);
    getLastImagePair(&imagePackets, &imagePair);
    double dayEnd = imagePair.secondsSince1970;
    // Filter images based on time of day if yyyymmdd was passed
    // Seconds since 1970
    if (dateToSecondsSince1970(satDate + 1, &dayStart))
    {
        fprintf(stderr, "Could not parse %s to a date.\n", satDate);
        goto cleanup;
    }
    dayEnd = dayStart + 86400.0; // ignore leap second on this day
 
    size_t numberOfImagePairs = countImagePairs(&imagePackets, &imagePair, dayStart, dayEnd);

    if (imagePairNumber > numberOfImagePairs)
    {
        fprintf(stderr, "Image pair %ld not found for %s\n", imagePairNumber, satDate);
        goto cleanup;
    }

    double maxH = requestedMax;
    double maxV = requestedMax;

    ImageAnomalies h;
    ImageAnomalies v;
    initializeAnomalyData(&h);
    initializeAnomalyData(&v);

    int pixelThreshold = 0;

    Image imageBuf;
    if (allocImage(&imageBuf, LOOK_IMAGE_WIDTH, LOOK_IMAGE_HEIGHT, 1) != DRAW_OK)
    {
        printf("Won't be able to remember the image, sorry.\n");
        goto cleanup;
    }

    int imagesRead = 0;
    getAlignedImagePair(&imagePackets, (imagePairNumber-1)*2, &imagePair, &imagesRead);

    // Raw image anomalies
    analyzeRawImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
    analyzeRawImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

    // Draw images
    memset(imageBuf.pixels, BACKGROUND_COLOR, imageBuf.numberOfBytes);
    maxH = getMaxValue(imagePair.pixelsH, requestedMax);
    maxV = getMaxValue(imagePair.pixelsV, requestedMax);
    drawImagePair(&imageBuf, &imagePair, maxH, maxV, LOOK_IMAGE_RAW_X0, LOOK_IMAGE_RAW_Y0, LOOK_IMAGE_SCALE, LOOK_IMAGE_SEPARATION, "Raw H", "Raw V", true, &identityFilter, NULL, NULL);

    // Image processing results
    // Angel's wing moments
    if (h.upperAngelsWingAnomaly)
    {
        int dxh = (int)round(sqrt(h.upperAngelsWingX2));
        int dyh = (int)round(sqrt(h.upperAngelsWingY2));
        int x1h = (int)round(h.upperAngelsWingX1);
        int y1h = (int)round(h.upperAngelsWingY1);
        imageHorizontalLine(&imageBuf, false, H_SENSOR, x1h - dxh, x1h + dxh, y1h, FOREGROUND_COLOR, 2);
        imageVerticalLine(&imageBuf, false, H_SENSOR, x1h, y1h - dyh, y1h + dyh, FOREGROUND_COLOR, 2);
    }
    if (h.lowerAngelsWingAnomaly)
    {
        int dxh = (int)round(sqrt(h.lowerAngelsWingX2));
        int dyh = (int)round(sqrt(h.lowerAngelsWingY2));
        int x1h = (int)round(h.lowerAngelsWingX1);
        int y1h = (int)round(h.lowerAngelsWingY1);
        imageHorizontalLine(&imageBuf, false, V_SENSOR, x1h - dxh, x1h + dxh, y1h, FOREGROUND_COLOR, 2);
        imageVerticalLine(&imageBuf, false, V_SENSOR, x1h, y1h - dyh, y1h + dyh, FOREGROUND_COLOR, 2);
    }
    if (v.upperAngelsWingAnomaly)
    {
        int dxv = (int)round(sqrt(v.upperAngelsWingX2));
        int dyv = (int)round(sqrt(v.upperAngelsWingY2));
        int x1v = (int)round(v.upperAngelsWingX1);
        int y1v = (int)round(v.upperAngelsWingY1);
        imageHorizontalLine(&imageBuf, false, V_SENSOR, x1v - dxv, x1v + dxv, y1v, FOREGROUND_COLOR, 2);
        imageVerticalLine(&imageBuf, false, V_SENSOR, x1v, y1v - dyv, y1v + dyv, FOREGROUND_COLOR, 2);
    }
    if (v.lowerAngelsWingAnomaly)
    {
        int dxv = (int)round(sqrt(v.lowerAngelsWingX2));
        int dyv = (int)round(sqrt(v.lowerAngelsWingY2));
        int x1v = (int)round(v.lowerAngelsWingX1);
        int y1v = (int)round(v.lowerAngelsWingY1);
        imageHorizontalLine(&imageBuf, false, V_SENSOR, x1v - dxv, x1v + dxv, y1v, FOREGROUND_COLOR, 2);
        imageVerticalLine(&imageBuf, false, V_SENSOR, x1v, y1v - dyv, y1v + dyv, FOREGROUND_COLOR, 2);
    }

    // Testing regions
    // // Lower Angel's wing H
    // imageRectangle(&imageBuf, false, H_SENSOR, 32, 38, 38, 50, BACKGROUND_COLOR);
    // // Upper Angel's wing H
    // imageRectangle(&imageBuf, false, H_SENSOR, 32, 38, 14, 26, BACKGROUND_COLOR);
    // // Lower Angel's wing V
    // imageRectangle(&imageBuf, false, V_SENSOR, 32, 38, 38, 50, BACKGROUND_COLOR);
    // // Upper Angel's wing V
    // imageRectangle(&imageBuf, false, V_SENSOR, 32, 38, 14, 26, BACKGROUND_COLOR);

    // Gain corrected image anomalies
    latestConfigValues(&imagePair, &timeSeries, &pixelThreshold, NULL, NULL, NULL, NULL, NULL, NULL);
    applyImagePairGainMaps(&imagePair, pixelThreshold, NULL, NULL);

    analyzeGainCorrectedImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
    analyzeGainCorrectedImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

    maxH = getMaxValue(imagePair.pixelsH, requestedMax);
    maxV = getMaxValue(imagePair.pixelsV, requestedMax);
    drawImagePair(&imageBuf, &imagePair, maxH, maxV, LOOK_IMAGE_GC_X0, LOOK_IMAGE_GC_Y0, LOOK_IMAGE_SCALE, LOOK_IMAGE_SEPARATION, "Corrected H", "Corrected V", false, &identityFilter, NULL, NULL);

    // draw image processing results

    // Nominal O+ Moments
    // Left hand column and top row are included in x and y values of moments.
    // Moments are calculated only on pixel data within a rectangular region
    if (h.x1 > 0.)
    {
        int dxh = (int)round(sqrt(h.x2));
        int dyh = (int)round(sqrt(h.y2));
        int x1h = (int)round(h.x1);
        int y1h = (int)round(h.y1);
        imageHorizontalLine(&imageBuf, true, H_SENSOR, x1h - dxh, x1h + dxh, y1h, FOREGROUND_COLOR, 2);
        imageVerticalLine(&imageBuf, true, H_SENSOR, x1h, y1h - dyh, y1h + dyh, FOREGROUND_COLOR, 2);
    }
    if (v.x1 > 0.)
    {
        int dxv = (int)round(sqrt(v.x2));
        int dyv = (int)round(sqrt(v.y2));
        int x1v = (int)round(v.x1);
        int y1v = (int)round(v.y1);
        imageHorizontalLine(&imageBuf, true, V_SENSOR, x1v - dxv, x1v + dxv, y1v, FOREGROUND_COLOR, 2);
        imageVerticalLine(&imageBuf, true, V_SENSOR, x1v, y1v - dyv, y1v + dyv, FOREGROUND_COLOR, 2);
    }
    

    // Export PNG
    char filename[FILENAME_MAX];
    sprintf(filename, "%s/%s_%06ld.png", outputDir, satDate, imagePairNumber);
    struct spng_plte colors = getColorTable();
    writePng(filename, &imageBuf, &colors);
    printf("%s\n", filename);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    freeLpTiiTimeSeries(&timeSeries);

    freeImage(&imageBuf);

    fflush(stdout);

    exit(0);
}


void usage(const char * name)
{
    printf("\nTII Look %s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd maximumCount imagePairNumber outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");
    printf("\n");
    printf("Pass -1 for maximumCount to autoscale each image.\n");
    printf("\n");
    printf("imagePairNumber starts at 1 for first TII image pair.\n");

    return;
}

void imagePoint(Image *image, bool correctedImage, int sensor, int columnFromLeft, int rowFromTop, int color, int size)
{
    if (columnFromLeft < 0 || columnFromLeft > TII_COLS - 1 || rowFromTop < 0 || rowFromTop > TII_ROWS - 1)
        return;
    
    int x0 = 0, y0 = 0;
    imageOrigin(correctedImage, sensor, &x0, &y0);
    drawPoint(image, x0 + LOOK_IMAGE_SCALE * columnFromLeft, y0 + LOOK_IMAGE_SCALE * rowFromTop, color, size);

}

void imageVerticalLine(Image *image, bool correctedImage, int sensor, int columnFromLeft, int startRowFromTop, int stopRowFromTop, int color, int size)
{
    if (columnFromLeft < 0 || columnFromLeft > TII_COLS - 1)
        return;

    int x0 = 0, y0 = 0;
    imageOrigin(correctedImage, sensor, &x0, &y0);

    for (int r = startRowFromTop; r >= 0 && r < stopRowFromTop && r < TII_ROWS; r++)
        for (int s = 0; s < LOOK_IMAGE_SCALE; s++)
            drawPoint(image, x0 + LOOK_IMAGE_SCALE * columnFromLeft, y0 + LOOK_IMAGE_SCALE * r + s, color, size);
}

void imageHorizontalLine(Image *image, bool correctedImage, int sensor, int startColumnFromLeft, int stopColumnFromLeft, int rowFromTop, int color, int size)
{
    if (rowFromTop < 0 || rowFromTop > TII_ROWS - 1)
        return;

    int x0 = 0, y0 = 0;
    imageOrigin(correctedImage, sensor, &x0, &y0);

    for (int c = startColumnFromLeft; c >= 0 && c < stopColumnFromLeft && c < TII_COLS; c++)
        for (int s = 0; s < LOOK_IMAGE_SCALE; s++)
            drawPoint(image, x0 + LOOK_IMAGE_SCALE * c + s, y0 + LOOK_IMAGE_SCALE * rowFromTop, color, size);
}

void imageRectangle(Image *image, bool correctedImage, int sensor, int startColumnFromLeft, int stopColumnFromLeft, int startRowFromTop, int stopRowFromTop, int color)
{
    int x0 = 0, y0 = 0;
    imageOrigin(correctedImage, sensor, &x0, &y0);

    for (int c = startColumnFromLeft; c >= 0 && c <= stopColumnFromLeft && c < TII_COLS; c++)
        for (int r =startRowFromTop; r >= 0 && r <= stopRowFromTop && r < TII_ROWS; r++)
            for (int sc = 0; sc < LOOK_IMAGE_SCALE; sc++)
                for (int sr = 0; sr < LOOK_IMAGE_SCALE; sr++)
                    drawPoint(image, x0 + LOOK_IMAGE_SCALE * c + sc, y0 + LOOK_IMAGE_SCALE * r + sr, color, 1);
}


void imageOrigin(bool correctedImage, int sensor, int *x0, int *y0)
{
    if (x0 == NULL || y0 == NULL)
        return;

    *x0 = LOOK_IMAGE_RAW_X0;
    *y0 = LOOK_IMAGE_RAW_Y0;
    if (correctedImage)
    {
        *x0 = LOOK_IMAGE_GC_X0;
        *y0 = LOOK_IMAGE_GC_Y0;
    }
    if (sensor == V_SENSOR)
    {
        *x0 += LOOK_IMAGE_SCALE * (TII_COLS + LOOK_IMAGE_SEPARATION);
    }

}
