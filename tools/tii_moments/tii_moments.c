#include "tii_moments.h"

#include "tii.h"

#include "isp.h"
#include "gainmap.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--gainmaps") == 0)
    {
        gainMapInfo('A');
        gainMapInfo('B');
        gainMapInfo('C');
        exit(0);
    }
    if (argc != 4)
    {
        usage(argv[0]);
        exit(0);
    }

    exit(0);

    int status = 0;

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);

    if (sourceLen != 9)
    {
        usage(argv[0]);
	    exit(1);
    }
    double max = -1.0;
    char *outputDir = argv[2];

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

    // Per image moments
    char dailyMomentFilename[FILENAME_MAX];
    sprintf(dailyMomentFilename, "%s/SW_EFI%s_imaging_anomaly_stats.txt", outputDir, satDate);
    FILE *dailyMomentsFile = fopen(dailyMomentFilename, "w");
    if (dailyMomentsFile == NULL)
    {
        fprintf(stderr, "Could not open imaging anomaly stats file.\n");
        goto cleanup;
    }

    ImageAnomalies h;
    ImageAnomalies v;

    int pixelThreshold = 0;

    int imagesRead = 0;

    // time in sec since 1970, PA H, PA V, measles H, measles V, classic wing H, classic wing V, angel's wing upper H, angel's wing upper V, angel's wing lower H, angel's wing lower V
    fprintf(dailyMomentsFile, "secondsSince1970 TotalH TotalV X1H X1V Y1H Y1V X2H X2V Y2H Y2V\n");
    for (size_t i = 0; i < numberOfImagePairs; i++)
    {

        getAlignedImagePair(&imagePackets, 2*i, &imagePair, &imagesRead);

        if (scienceMode(imagePair.auxH) && scienceMode(imagePair.auxV))
        {
            initializeAnomalyData(&h);
            initializeAnomalyData(&v);

            // Raw image anomalies
            analyzeRawImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
            analyzeRawImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

            // Gain corrected image anomalies
            latestConfigValues(&imagePair, &timeSeries, &pixelThreshold, NULL, NULL, NULL, NULL, NULL, NULL);
            applyImagePairGainMaps(&imagePair, pixelThreshold, NULL, NULL);
            analyzeGainCorrectedImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
            analyzeGainCorrectedImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

            // time in sec since 1970, PA H, PA V, measles H, measles V, classic wing H, classic wing V, angel's wing upper H, angel's wing upper V, angel's wing lower H, angel's wing lower V
            fprintf(dailyMomentsFile, "%ld %d %d %d %d %d %d %d %d %d %d %d %d %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf\n", (time_t)floor(imagePair.secondsSince1970), h.peripheralAnomaly, v.peripheralAnomaly, h.measlesAnomaly, v.measlesAnomaly, h.classicWingAnomaly, v.classicWingAnomaly, h.bifurcationAnomaly, v.bifurcationAnomaly, h.upperAngelsWingAnomaly, v.upperAngelsWingAnomaly, h.lowerAngelsWingAnomaly, v.lowerAngelsWingAnomaly, h.upperAngelsWingX1, v.upperAngelsWingX1, h.upperAngelsWingY1, v.upperAngelsWingY1, h.upperAngelsWingX2, v.upperAngelsWingX2, h.upperAngelsWingY2, v.upperAngelsWingY2, h.lowerAngelsWingX1, v.lowerAngelsWingX1, h.lowerAngelsWingY1, v.lowerAngelsWingY1, h.lowerAngelsWingX2, v.lowerAngelsWingX2, h.lowerAngelsWingY2, v.lowerAngelsWingY2);
        }
    }
    fclose(dailyMomentsFile);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    freeLpTiiTimeSeries(&timeSeries);

    fflush(stdout);

    exit(0);
}

void gainMapInfo(const char satellite)
{
    int nMaps = 0;
    double *mapTimes = NULL;
    time_t seconds = 0;

    gainMapTimes(satellite, &nMaps, &mapTimes);

    struct tm *date;

    if (nMaps > 0)
    {
        printf("Gain map table for Swarm %c\n", satellite);
        printf("MapID\tDate uploaded\n");
        for (int i = 0; i < nMaps; i++)
        {
            seconds = (time_t) floor(mapTimes[i]);
            date = gmtime(&seconds);
            printf("%4d\t%4d%02d%02dT%02d%02d%02d UT\n", i, date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
        }
    }

    return;
}

void usage(const char * name)
{
    printf("\nTII Daily Image Moments %s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd GainMapId outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files. GainMapId is the gain map pair index to use, or -1 to automatically apply maps based on date uploaded to satellite.\n");
    printf("\n  %s --gainmaps lists gainmap dates and their IDs\n", name);

    return;
}

