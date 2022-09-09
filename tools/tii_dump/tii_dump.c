/*

    TIIM processing tools: tools/tii_dump/tii_dump.c

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

#include "tii_dump.h"

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

    size_t nTiiParameters = 10;
    TiiParameter tiiParameters[] = 
    {
        // Science 2 Hz
        {1, PARAM_2HZ, PARAM_DOUBLE, "L1aDensity1", "Provisional Langmuir probe density estimate from probe 1.", 0},
        {2, PARAM_2HZ, PARAM_DOUBLE, "L1aDensity2", "Provisional Langmuir probe density estimate from probe 1.", 1*sizeof(double)},
        {3, PARAM_2HZ, PARAM_DOUBLE, "y2H", "2nd y moment H sensor.", 2*sizeof(double)},
        {4, PARAM_2HZ, PARAM_DOUBLE, "y2V", "2nd y moment V sensor.", 3*sizeof(double)},
        {5, PARAM_2HZ, PARAM_DOUBLE, "VBiasSettingH", "Bias grid voltage setting H sensor", 4*sizeof(double)},
        {6, PARAM_2HZ, PARAM_DOUBLE, "VBiasSettingV", "Bias grid voltage setting V sensor", 5*sizeof(double)},
        {7, PARAM_2HZ, PARAM_DOUBLE, "VMcpSettingH", "MCP front voltage setting H sensor", 6*sizeof(double)},
        {8, PARAM_2HZ, PARAM_DOUBLE, "VMcpSettingV", "MCP front voltage setting V sensor", 7*sizeof(double)},
        {9, PARAM_2HZ, PARAM_DOUBLE, "VPhosSettingH", "Phosphor voltage setting H sensor", 8*sizeof(double)},
        {10, PARAM_2HZ, PARAM_DOUBLE, "VPhosSettingV", "Phosphor voltage setting V sensor", 9*sizeof(double)}
};

    if (argc > 1 && strcmp(argv[1], "--parameters") == 0)
    {
        parameterList(nTiiParameters, tiiParameters);
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

    char *parameterList = argv[2];
    int parameterCount = 1;
    for (int i = 0; i < strlen(parameterList); i++)
    {
        if (parameterList[i] == ',')
            parameterCount++;
    }
    int *parameterIds = (int *)malloc(parameterCount);
    if (parameterIds == NULL)
    {
        printf("Could not allocate heap memory.\n");
        exit(1);
    }
    char *tok = strtok(parameterList, ",");
    int n = 0;
    while (tok != NULL)
    {
        parameterIds[n++] = atoi(tok);
        tok = strtok(NULL, ",");
    } 
    // printf("Requested parameters\n");
    // printf("ID\tName\n");
    int nInvalidParameters = 0;
    for (int i = 0; i < parameterCount; i++)
    {
        if (!(parameterIds[i] > 0 && parameterIds[i] <= nTiiParameters))
            nInvalidParameters++;
    }
    if (nInvalidParameters > 0)
    {
        printf("%d invalid parameter ID%s specified.\n", nInvalidParameters, nInvalidParameters == 1 ? " was" : "s were");
        exit(1);
    }

    char *outputDir = argv[3];



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
    if (timeSeries.n2Hz == 0)
    {
        printf("No science data.");
        exit(1);
    }

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


    char dailyParameterFilename[FILENAME_MAX];
    sprintf(dailyParameterFilename, "%s/SW_EFI%s_parameter_stats.txt", outputDir, satDate);
    FILE *dailyParameterFile = fopen(dailyParameterFilename, "w");
    if (dailyParameterFile == NULL)
    {
        fprintf(stderr, "Could not open parameter stats file for writing.\n");
        goto cleanup;
    }

    ImageAnomalies h;
    ImageAnomalies v;

    int imagesRead = 0;

    // time in sec since 1970, PA H, PA V, measles H, measles V, classic wing H, classic wing V, angel's wing upper H, angel's wing upper V, angel's wing lower H, angel's wing lower V
    fprintf(dailyParameterFile, "Row format: <secondsSince1970>");
    for (int i = 0; i < parameterCount; i++)
    {
        fprintf(dailyParameterFile, " <");
        writeWithSpaceAsUnderscore(dailyParameterFile, tiiParameters[parameterIds[i]-1].name);
        fprintf(dailyParameterFile, ">");
    }
    fprintf(dailyParameterFile, "\n");

    size_t lastScienceIndex = 0;
    double lastScienceTime = timeSeries.lpTiiTime2Hz[0];
    for (size_t i = 0; i < numberOfImagePairs; i++)
    {

        getAlignedImagePair(&imagePackets, 2*i, &imagePair, &imagesRead);

        if (scienceMode(imagePair.auxH) && scienceMode(imagePair.auxV))
        {
            fprintf(dailyParameterFile, "%.0lf", lastScienceTime);
            // Get next parameter value corresponding to image time.
            for (int p = 0; p < parameterCount; p++)
            {
                switch (tiiParameters[parameterIds[p]-1].packetType)
                {
                    case PARAM_2HZ:
                        while (lastScienceTime < imagePair.secondsSince1970 && lastScienceIndex < timeSeries.n2Hz)
                        {
                            lastScienceIndex++;
                            lastScienceTime = timeSeries.lpTiiTime2Hz[lastScienceIndex];
                        }
                        // Take the next parameter if it was sampled within 1 s of the image time
                        if (fabs(lastScienceTime - imagePair.secondsSince1970) < 1.0)
                        {
                            double value = (*((double**)(((char*)&timeSeries.ionDensity1) + tiiParameters[parameterIds[p]-1].offset)))[lastScienceIndex];
                            fprintf(dailyParameterFile, " %lf", value);
                        }
                        else
                            fprintf(dailyParameterFile, " -");
                        break;
                    default:
                        break;
                }
            }
            fprintf(dailyParameterFile, "\n");
        }
    }
    fclose(dailyParameterFile);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    freeLpTiiTimeSeries(&timeSeries);
    free(parameterIds);

    fflush(stdout);


    exit(0);
}

void parameterList(size_t nParameters, TiiParameter *parameters)
{

    printf("TII parameter table\n");
    printf("ID\tName\n");

    for (int i = 0; i < nParameters; i++)
    {
        printf("%4d\t%s\n", parameters[i].id, parameters[i].name);
    }

    return;
}

void usage(const char * name)
{
    printf("\nTII Parameter Dump %s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd parameterIds outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files. parameterIds is a comma-separated list of parameters to dump.\n");
    printf("\n  %s --parameters lists the available parameters IDs\n", name);

    return;
}

void writeWithSpaceAsUnderscore(FILE *fd, char *s)
{
    char *p = s;
    while (*p)
    {
        if (*p == ' ' || *p == '\t')
            fprintf(fd, "_");
        else
            fprintf(fd, "%c", *p);
        p++;
    }

    return;
}