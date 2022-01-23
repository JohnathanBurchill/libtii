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
    double max = atof(argv[2]);
    long imagePairNumber = atol(argv[3]);
    if (imagePairNumber < 1)
    {
        usage(argv[0]);
        exit(1);
    }
    char *outputDir = argv[4];

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
