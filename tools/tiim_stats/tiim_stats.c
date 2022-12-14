/*

    TIIM processing tools: tools/tiim_stats/tiim_stats.c

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

#include "tii.h"

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

void statsusage(const char * name);

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

    ImagePairTimeSeries imagePairTimeSeries;
    initImagePairTimeSeries(&imagePairTimeSeries);

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

    // Summary
    // start time (sec from 1970), end time, satLetter, imagePairs, measlesCountH, measlesCountV, paCumulativeFrameCountH, paCumulativeFrameCountV, paAngularFrameCountsH... paAngularFramecountsV...

    // TODO export to separate file the daily summary
    // size_t n = imagePairTimeSeries.nImagePairs;
    // if (n > 0)
    // {
    //     printf("%ld %ld %c %ld %d %d %d %d", (time_t)floor(imagePairTimeSeries.time[0]), (time_t)floor(imagePairTimeSeries.time[n-1]), imagePairTimeSeries.satellite, imagePairTimeSeries.nImagePairs, imagePairTimeSeries.cumulativeMeaslesCountH[n-1], imagePairTimeSeries.cumulativeMeaslesCountV[n-1], imagePairTimeSeries.paCumulativeFrameCountH[n-1], imagePairTimeSeries.paCumulativeFrameCountH[n-1]);

    //     printf(" %d", PA_ANGULAR_NUM_BINS);
    //     for (int i = 0; i < PA_ANGULAR_NUM_BINS; i++)
    //     {
    //         printf(" %d", imagePairTimeSeries.paAngularSpectrumCumulativeFrameCountH[(n-1)*PA_ANGULAR_NUM_BINS + i]);
    //     }
    //     for (int i = 0; i < PA_ANGULAR_NUM_BINS; i++)
    //     {
    //         printf(" %d", imagePairTimeSeries.paAngularSpectrumCumulativeFrameCountV[(n-1)*PA_ANGULAR_NUM_BINS + i]);
    //     }
    //     printf("\n");
    // }

    // Per image measles and PA stats
    char measlesPaFilename[FILENAME_MAX];
    sprintf(measlesPaFilename, "%s/SW_EFI%s_image_stats.txt", outputDir, satDate);
    FILE *measlesPaFile = fopen(measlesPaFilename, "w");
    if (measlesPaFile == NULL)
    {
        fprintf(stderr, "Could not open stats file.\n");
        goto cleanup;
    }

    for (size_t i = 0; i < numberOfImagePairs; i++)
    {
        // time in sec since 1970, measles count H, measles count V, PA count H, PA count V, VPhos H, VPhosV, VMcp H, VMcp V, VBias H, VBias V, VFP H
        fprintf(measlesPaFile, "%ld %d %d %d %d %f %f %f %f %f %f %f\n", (time_t)floor(imagePairTimeSeries.time[i]), imagePairTimeSeries.measlesCountH[i], imagePairTimeSeries.measlesCountV[i], imagePairTimeSeries.paCountH[i], imagePairTimeSeries.paCountV[i], imagePairTimeSeries.PhosphorVoltageMonitorH[i], imagePairTimeSeries.PhosphorVoltageMonitorV[i], imagePairTimeSeries.McpVoltageMonitorH[i], imagePairTimeSeries.McpVoltageMonitorV[i], imagePairTimeSeries.BiasGridVoltageMonitorH[i], imagePairTimeSeries.BiasGridVoltageMonitorV[i], imagePairTimeSeries.FaceplateVoltageMonitorH[i]);
    }
    fclose(measlesPaFile);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);

    freeImagePairTimeSeries(&imagePairTimeSeries);

    fflush(stdout);

    exit(0);
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
