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
    if (argc > 2 && strcmp(argv[1], "--gainmaps") == 0)
    {
        gainMapInfo(argv[2][0]);
        exit(0);
    }
    if (argc != 4)
    {
        usage(argv[0]);
        exit(0);
    }

    int status = 0;

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);

    if (sourceLen != 9)
    {
        usage(argv[0]);
	    exit(1);
    }

    char satellite = satDate[0];
    if (satellite != 'A' && satellite != 'B' && satellite != 'C')
    {
        printf("Invalid satellite letter %c. Expected A, B, or C\n", satellite);
        exit(1);
    }

    double max = -1.0;
    int gainMapId = atoi(argv[2]);
    char *outputDir = argv[3];


    // Check valid gain map ID
    int nGainMaps = 0;
    double *gainMapTimeArray = NULL;
    status = gainMapTimes(satellite, &nGainMaps, &gainMapTimeArray);
    if (status != 0)
    {
        printf("Unable to load gain map times for Swarm %d\n", satellite);
        exit(0);
    }

    if (gainMapId < -1 || gainMapId > nGainMaps)
    {
        printf("Invalid gain map ID (=%d) for Swarm %c, must be -1 (no processing), 0 (nominal gain map processing), or between 1 and %d (processing with gain map ID below.\n", gainMapId, satellite, nGainMaps);
        gainMapInfo(satellite);
        exit(1);
    }

    double gainMapTime = 0.0;
    if (gainMapId > 0)
    {
        gainMapTime = gainMapTimeArray[gainMapId-1]; 
    }

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

    char gainCorrectionString[255];
    if (gainMapId == -1)
        sprintf(gainCorrectionString, "_gain_correction_none");
    else if (gainMapId == 0)
        sprintf(gainCorrectionString, "_gain_correction_nominal");
    else
    {
        time_t seconds = (time_t) floor(gainMapTimeArray[gainMapId-1]);
        struct tm *date = gmtime(&seconds);
        sprintf(gainCorrectionString, "_gain_correction_map_id_%d_uploaded_%04d%02d%02dT%02d%02d%02dUT", gainMapId, date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
    }

    char dailyMomentFilename[FILENAME_MAX];
    sprintf(dailyMomentFilename, "%s/SW_EFI%s_moment_stats_%s.txt", outputDir, satDate, gainCorrectionString);
    FILE *dailyMomentsFile = fopen(dailyMomentFilename, "w");
    if (dailyMomentsFile == NULL)
    {
        fprintf(stderr, "Could not open imaging anomaly stats file.\n");
        goto cleanup;
    }

    ImageAnomalies h;
    ImageAnomalies v;

    // Default onboard processing parameters
    int pixelThreshold = 0;
    int minCol = 33;
    int maxCol = 64;
    int nCols = 32;

    // indexing is for sensor [H|V]
    double total[2] = {0};
    double x1[2] = {0.0};
    double y1[2] = {0.0};
    double agcControl[2] = {0.0};

    int imagesRead = 0;

    // time in sec since 1970, PA H, PA V, measles H, measles V, classic wing H, classic wing V, angel's wing upper H, angel's wing upper V, angel's wing lower H, angel's wing lower V
    fprintf(dailyMomentsFile, "secondsSince1970 TotalH TotalV X1H X1V Y1H Y1V AgcControlH AgcControlV\n");
    for (size_t i = 0; i < numberOfImagePairs; i++)
    {

        getAlignedImagePair(&imagePackets, 2*i, &imagePair, &imagesRead);

        if (scienceMode(imagePair.auxH) && scienceMode(imagePair.auxV))
        {
            latestConfigValues(&imagePair, &timeSeries, &pixelThreshold, &minCol, &maxCol, &nCols, NULL, NULL, NULL);

            // Defaults to processing the raw image (no gain correction)
            // (i.e., gainMapId == -1)

            if (gainMapId >= 0)
            {
                // Unless gainmap corrections are requested
                // Defaults to nominal processing with gainmap as uploaded to instrument
                if (gainMapId > 0)
                {
                    // Unless a specific gainmap ID is requested
                    imagePair.auxH->dateTime.secondsSince1970 = gainMapTime + 1.0;
                    imagePair.auxV->dateTime.secondsSince1970 = gainMapTime + 1.0;
                }
                applyImagePairGainMaps(&imagePair, pixelThreshold, NULL, NULL);
            }
            onboardProcessing(imagePair.pixelsH, imagePair.gotImageH, minCol, maxCol, nCols, &total[0], &x1[0], &y1[0], &agcControl[0]);
            onboardProcessing(imagePair.pixelsV, imagePair.gotImageV, minCol, maxCol, nCols, &total[1], &x1[1], &y1[1], &agcControl[1]);

            // time in sec since 1970, PA H, PA V, measles H, measles V, classic wing H, classic wing V, angel's wing upper H, angel's wing upper V, angel's wing lower H, angel's wing lower V
            fprintf(dailyMomentsFile, "%ld %.1lf %.1lf %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf\n", (time_t)floor(imagePair.secondsSince1970), total[0], total[1], x1[0], x1[1], y1[0], y1[1], agcControl[0], agcControl[1]);
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

void usage(const char * name)
{
    printf("\nTII Daily Image Moments %s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd GainMapId outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files. GainMapId is the gain map pair index to use, or -1 to automatically apply maps based on date uploaded to satellite.\n");
    printf("\n  %s --gainmaps <satelliteLetter> lists gainmap IDs and the dates they were loaded to the instrument for the specified satellite (A, B or C)\n", name);

    return;
}

