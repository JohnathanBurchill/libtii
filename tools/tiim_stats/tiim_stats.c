/*

    TIIM processing tools: tools/tiim_stats/tiim_stats.c

    Copyright (C) 2025  Johnathan K Burchill

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

#include "tii/tii.h"
#include "tii/isp.h"
#include "tii/import.h"
#include "tii/utility.h"
#include "tii/analysis.h"
#include "tii/timeseries.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

void statsusage(const char * name);

void gsubf(double t, LpTiiTimeSeries *timeSeries, int *lpIndex, int agcLower, double agcH, double agcV, double *meanDensity, double *gfH, double *gfV);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        statsusage(argv[0]);
        exit(1);
    }

    int status = 0;

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);

    // Daily image statistics only
    if (sourceLen != 9)
    {
        statsusage(argv[0]);
	    exit(1);
    }
    double max = -1.0;
    char *outputDir = argv[2];

    // Data
    ImagePackets imagePackets;
    SciencePackets sciencePackets = {0};

    ImagePairTimeSeries imagePairTimeSeries;
    initImagePairTimeSeries(&imagePairTimeSeries);
    LpTiiTimeSeries timeSeries = {0};
    initLpTiiTimeSeries(&timeSeries);

    status = importImagery(satDate, ".", &imagePackets);
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

    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS], pixelsV[NUM_FULL_IMAGE_PIXELS];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImagePair imagePair;
    ImageAuxData auxH, auxV;
    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    double maxValueH, maxValueV;
    int imagesRead = 0;
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
    getImagePairTimeSeries(satellite, &imagePackets, &imagePair, &imagePairTimeSeries, numberOfImagePairs, dayStart, dayEnd, max);

    importScience(satDate, ".", &sciencePackets);
    getLpTiiTimeSeries(satellite, &sciencePackets, &timeSeries);

    // Per image measles and PA stats
    char measlesPaFilename[FILENAME_MAX];
    sprintf(measlesPaFilename, "%s/SW_EFI%s_image_stats.txt", outputDir, satDate);
    FILE *measlesPaFile = fopen(measlesPaFilename, "w");
    if (measlesPaFile == NULL)
    {
        fprintf(stderr, "Could not open stats file.\n");
        goto cleanup;
    }

    int vshHSetting = 0;
    int vshVSetting = 0;
    double vshH = 0.0;
    double vshV = 0.0;

    double agcH = 0.0;
    double agcV = 0.0;
    int lpInd = 0;
    double meanDensity = 0.0;
    double gfH = 0.0;
    double gfV = 0.0;

    for (size_t i = 0; i < numberOfImagePairs; /* INCREMENT HANDLED BELOW! */) {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);
        if (status == ISP_NO_IMAGE_PAIR)
        {
            i++;
            continue;
        }
        i+=imagesRead;

        int pixelthreshold = 0;
        int mincol = 0;
        int maxcol = 0;
        int ncols = 0;
        bool agcenabled = false;
        int agclower = 0;
        int agcupper = 0;
        latestConfigValues(imagePair.secondsSince1970, &timeSeries, &pixelthreshold, &mincol, &maxcol, &ncols, &agcenabled, &agclower, &agcupper, &vshHSetting, &vshVSetting, NULL, NULL, NULL, NULL);
        applyImagePairGainMaps(&imagePair, pixelthreshold, NULL, NULL);
        onboardProcessing(imagePair.pixelsH, imagePair.gotImageH, mincol, maxcol, ncols, &imagePairTimeSeries.totalCountsH[i], &imagePairTimeSeries.x1H[i], &imagePairTimeSeries.y1H[i], &imagePairTimeSeries.agcControlValueH[i]);
        onboardProcessing(imagePair.pixelsV, imagePair.gotImageV, mincol, maxcol, ncols, &imagePairTimeSeries.totalCountsV[i], &imagePairTimeSeries.x1V[i], &imagePairTimeSeries.y1V[i], &imagePairTimeSeries.agcControlValueV[i]);
        agcH = imagePairTimeSeries.agcControlValueH[i];
        agcV = imagePairTimeSeries.agcControlValueV[i];
        gsubf(imagePair.secondsSince1970, &timeSeries, &lpInd, agclower, agcH, agcV, &meanDensity, &gfH, &gfV);

        vshH = -100.0 * (double)vshHSetting / 255.0;
        vshV = -100.0 * (double)vshVSetting / 255.0;

        // time in sec since 1970, measles count H, measles count V, PA count H, PA count V, VPhos H, VPhosV, VMcp H, VMcp V, VBias H, VBias V, VFP H, VSh H, VSh V
        fprintf(measlesPaFile, "%ld %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f\n", (time_t)floor(imagePairTimeSeries.time[i]), imagePairTimeSeries.measlesCountH[i], imagePairTimeSeries.measlesCountV[i], imagePairTimeSeries.paCountH[i], imagePairTimeSeries.paCountV[i], imagePairTimeSeries.PhosphorVoltageMonitorH[i], imagePairTimeSeries.PhosphorVoltageMonitorV[i], imagePairTimeSeries.McpVoltageMonitorH[i], imagePairTimeSeries.McpVoltageMonitorV[i], imagePairTimeSeries.BiasGridVoltageMonitorH[i], imagePairTimeSeries.BiasGridVoltageMonitorV[i], imagePairTimeSeries.FaceplateVoltageMonitorH[i], vshH, vshV, gfH, gfV);

    }
    fclose(measlesPaFile);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);

    freeImagePairTimeSeries(&imagePairTimeSeries);

    fflush(stdout);

    exit(0);
}

void gsubf(double t, LpTiiTimeSeries *timeSeries, int *lpIndex, int agcLower, double agcH, double agcV, double *meanDensity, double *gfH, double *gfV) {

    // Estimate mean ion density leading up to this image
    int lpInd = *lpIndex;
    double lpTime = timeSeries->lpTiiTime2Hz[lpInd];
    double ni = timeSeries->ionDensity2[lpInd];
    double meanni = 0.0;
    double nni = 0.0;
    double meangainH = 0.0;
    double meangainV = 0.0;
    double ngainH = 0.0;
    double ngainV = 0.0;
    while(lpTime < t && lpInd < timeSeries->n2Hz - 1) {
        ++lpInd;
        lpTime = timeSeries->lpTiiTime2Hz[lpInd];
        ni = timeSeries->ionDensity2[lpInd];
        meanni += ni;
        ++nni;
    }
    *lpIndex = lpInd;
    if (nni > 0) {
        meanni /= nni;
    } else {
        meanni = 0;
    }
    *meanDensity = meanni;

    if (meanni > 0 && agcH > 0 && agcH < agcLower) {
        *gfH = agcH / meanni;
    } else {
        *gfH = 0.0;
    }
    if (meanni > 0 && agcV > 0 && agcV < agcLower) {
        *gfV = agcV / meanni;
    } else {
        *gfV = 0.0;
    }

    return;

}

void statsusage(const char * name)
{
    printf("\nTII Daily Image Statistics %s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");

    return;
}
