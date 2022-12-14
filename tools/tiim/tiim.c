/*

    TIIM processing tools: tools/tiim/tiim.c

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

#include "tiim.h"

#include "tiigraphics.h"
#include "colors.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include "draw.h"
#include "fonts.h"
#include "video.h"
#include "filters.h"

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
    if (argc < 4 || argc > 5)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int status = 0;
    char movieFilename[FILENAME_MAX];
    int frameCounter = 0;
    int gotImagePairTimeSeries = 0;
    int gotLpTiiTimeSeries = 0;

    char densityText[255];

    char * hdr = argv[1];
    size_t sourceLen = strlen(hdr);

    if (strcmp(hdr + sourceLen - 4, ".HDR") != 0 && sourceLen != 9)
    {
        usage(argv[0]);
	exit(EXIT_FAILURE);
    }
    double max = atof(argv[2]);
    char *outputDir = argv[3];

    bool overwrite = false;
    for (int i = 4; i < argc; i++)
    {
        char *arg = argv[i];
        if (strcmp(arg, "-f") == 0)
            overwrite = true;
    }

    // Data
    ImagePackets imagePackets = {0};
    SciencePackets sciencePackets = {0};
    Image templateImage = {0};
    Image image = {0};

    LpTiiTimeSeries timeSeries = {0};
    initLpTiiTimeSeries(&timeSeries);
    ImagePairTimeSeries imagePairTimeSeries = {0};
    initImagePairTimeSeries(&imagePairTimeSeries);

    if (allocImage(&templateImage, IMAGE_WIDTH, IMAGE_HEIGHT, 1) != DRAW_OK)
    {
        printf("Could not allocate memory for template image.\n");
        goto cleanup;
    }    
    if (allocImage(&image, IMAGE_WIDTH, IMAGE_HEIGHT, 1) != DRAW_OK)
    {
        printf("Could not allocate memory for image.\n");
        goto cleanup;
    }    
    status = importImagery(hdr, &imagePackets);
    if (status)
    {
	if (status == IMPORT_NO_RECORDS && sourceLen == 9)
	{
	    // Print only for daily file generation. This
	    // message is already issued on a per HDR file basis.
	    fprintf(stderr, "No records found for %s\n", hdr);
	}
	else if (status != IMPORT_NO_RECORDS)
	{
            fprintf(stderr, "Could not import image data: error %d.\n", status);
	}
        goto cleanup;
    }
    if (imagePackets.numberOfImages == 0)
    {
        if (sourceLen == 9)
            fprintf(stderr, "No images found for satellite %c for date %s\n", hdr[0], hdr+1);
        else
            fprintf(stderr, "No images found in file %s\n", hdr);
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
    if (sourceLen == 9)
    {
        // Seconds since 1970
        if (dateToSecondsSince1970(hdr + 1, &dayStart))
        {
            printf("Could not parse %s to a date.\n", hdr);
            goto cleanup;
        }
        dayEnd = dayStart + 86400.0; // ignore leap second on this day
    }

    status = constructMovieFilename(satellite, (size_t)dayStart, (size_t)(dayEnd-1), outputDir, movieFilename);
    if (status != UTIL_OK)
    {
        printf("Could not construct movie filename.\n");
        goto cleanup;
    }

    if (!access(movieFilename, F_OK) && !overwrite)
    {
        printf("%s exists, skipping. Append -f option to force export.\n", movieFilename);
        goto cleanup;
    }

    status = initVideo(movieFilename);
    if (status < 0)
    {
        fprintf(stderr, "Problem intializing video: got status %d.\n", status);
        goto cleanup;
    }

    // Construct frames and export MPEG
    size_t numberOfImagePairs = countImagePairs(&imagePackets, &imagePair, dayStart, dayEnd);
    getImagePairTimeSeries(satellite, &imagePackets, &imagePair, &imagePairTimeSeries, numberOfImagePairs, dayStart, dayEnd, max);

    // Ignore result status, as we will display imagery whether or not there are science packets
    importScience(hdr, &sciencePackets);
    getLpTiiTimeSeries(satellite, &sciencePackets, &timeSeries);

    // Static content from frame to frame
    bool fullDay = sourceLen == 9;
    drawTemplate(&templateImage, &timeSeries, &imagePairTimeSeries, dayStart, dayEnd, fullDay);

    int nImagePairs = 0;

    long lastScienceIndex = 0;
    double lastScienceTime = 0.0;

    for (size_t i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        if (status == ISP_NO_IMAGE_PAIR)
        {
            // Images read would be 0, so force advance by one
            i++;
            continue;
        }
        i+=imagesRead;

        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd))
            continue;

        drawFrame(&image, &templateImage, &imagePair, &timeSeries, &imagePairTimeSeries, nImagePairs, frameCounter, dayStart, dayEnd);
        // Add Ion density 
        annotate("  Density:", 9, MONITOR_LABEL_OFFSET_X, 50 + 5 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, &image);
        while (lastScienceIndex < timeSeries.n2Hz && lastScienceTime < imagePair.secondsSince1970)
        {
            lastScienceIndex++;
            if (lastScienceIndex < timeSeries.n2Hz)
                lastScienceTime = timeSeries.lpTiiTime2Hz[lastScienceIndex];
        }
        if (lastScienceIndex < timeSeries.n2Hz && fabs(lastScienceTime - imagePair.secondsSince1970)< 120.0)
        {
            // annotate the ion density
            sprintf(densityText, "  %7.0lf cm^-3", timeSeries.ionDensity2[lastScienceIndex]);
            annotate(densityText, 9, MONITOR_LABEL_OFFSET_X + 80, 50 + 5 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, &image);
        }
        else {
            // Not available
            annotate("not available", 9, MONITOR_LABEL_OFFSET_X + 115, 50 + 5 * LINE_SPACING + MONITOR_LABEL_OFFSET_Y, &image);
        }

        generateFrame(&image, frameCounter);

        frameCounter++;
        nImagePairs++;


    }
    // Allow a moment before continuing with frame analysis
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(&image, frameCounter++);

    int nAcross = 16;
    int nDown = 8;
    int dx = 42;
    int dy = 67;
    int xborder = IMAGE_WIDTH / 2 - nAcross / 2 * dx;
    int yborder=3;

    // Frame-by-frame H
    int xoffset = 0;
    int yoffset = 0;
    insertTransition(&image, "Frame-by-frame Horizontal sensor", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);

    nImagePairs = 0;
    for (size_t i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        if (status == ISP_NO_IMAGE_PAIR)
        {
            i++;
            continue;
        }
        i+=imagesRead;

        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd))
            continue;

        if (xoffset == 0 && imagePair.gotImageH)
            drawTimestamp(&image, 15, yborder + yoffset * dy + 25, imagePair.auxH, 12);
        if (xoffset == nAcross-1 && imagePair.gotImageH)
            drawTimestamp(&image, IMAGE_WIDTH - 8*8-60, yborder + yoffset * dy+25, imagePair.auxH, 12);
            
        drawImage(&image, imagePair.pixelsH, imagePair.gotImageH, imagePairTimeSeries.maxValueH[nImagePairs], xborder+(xoffset++)*dx, yborder+yoffset*dy, 1, false, imagePair.auxH, &identityFilter, NULL);
        xoffset %= nAcross;
        if (xoffset == 0) yoffset++;
        // Show for 3 seconds each
        if (xoffset == 0 && yoffset == nDown)
        {
            for (int c = 0; c < 3 * VIDEO_FPS; c++)
                generateFrame(&image, frameCounter++);
            xoffset = 0;
            yoffset = 0;
            drawFill(&image, BACKGROUND_COLOR);
        }
        nImagePairs++;
    }
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(&image, frameCounter++);

    // Frame-by-frame V
    xoffset = 0;
    yoffset = 0;
    insertTransition(&image, "Frame-by-frame Vertical sensor", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);
    nImagePairs= 0;
    for (size_t i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        if (status == ISP_NO_IMAGE_PAIR)
        {
            i++;
            continue;
        }
        i+=imagesRead;

        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd))
            continue;


        if (xoffset == 0 && imagePair.gotImageV)
            drawTimestamp(&image, 15, yborder + yoffset * dy + 25, imagePair.auxV, 12);
        if (xoffset == nAcross-1 && imagePair.gotImageV)
            drawTimestamp(&image, IMAGE_WIDTH - 8*8-60, yborder + yoffset * dy+25, imagePair.auxV, 12);

        drawImage(&image, imagePair.pixelsV, imagePair.gotImageV, imagePairTimeSeries.maxValueV[nImagePairs], xborder+(xoffset++)*dx, yborder+yoffset*dy, 1, false, imagePair.auxV, &identityFilter, NULL);
        xoffset %= nAcross;
        if (xoffset == 0) yoffset++;
        // Show for 3 seconds each
        if (xoffset == 0 && yoffset == nDown)
        {
            for (int c = 0; c < 3 * VIDEO_FPS; c++)
                generateFrame(&image, frameCounter++);
            xoffset = 0;
            yoffset = 0;
            drawFill(&image, BACKGROUND_COLOR);
        }
        nImagePairs++;
    }
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(&image, frameCounter++);

    // Draw PA and measles time series
    int plotWidth = 500;
    int plotHeight = 100;
    int ox = 100;
    int oy = 140;
    int dotSize = 2;
    char xlabel[255];
    sprintf(xlabel, "%s", "Hours from start of file");
    if (sourceLen == 9)
    {
        dotSize = 2; // full day
        sprintf(xlabel, "%s", "UT hours");
    }
     
    insertTransition(&image, "Anomaly overview", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);
    drawIntTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.paCountH, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 1000, "", "PA Level", 1, MAX_COLOR_VALUE + 1, "0", "1000", false, dotSize, 12, true);
    drawIntTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.paCountV, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 1000, "", "", 1, 13, "", "", false, dotSize, 12, false);
    
    drawIntTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.measlesCountH, nImagePairs, ox, oy + plotHeight + 50, plotWidth, plotHeight, dayStart, dayEnd, 0, 200, xlabel, "Measles Level", 1, MAX_COLOR_VALUE + 1, "0", "200", false, dotSize, 12, true);
    drawIntTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.measlesCountV, nImagePairs, ox, oy + plotHeight + 50, plotWidth, plotHeight, dayStart, dayEnd, 0, 200, "", "", 1, 13, "", "", false, dotSize, 12, false);
    for (int c = 0; c < 3.0 * VIDEO_FPS; c++)
        generateFrame(&image, frameCounter++);

    // Onboard processing review
    plotWidth = 500;
    plotHeight = 80;
    ox = 20;
    oy = 110;
    dy = 35;
    insertTransition(&image, "Onboard processing overview", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);
    drawIntTimeSeries(&image, timeSeries.configTime, timeSeries.agcLowerThresholdConfig, timeSeries.nConfig, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "", 1, MAX_COLOR_VALUE + 2, "0", "", false, 1, 12, false);
    drawIntTimeSeries(&image, timeSeries.configTime, timeSeries.agcUpperThresholdConfig, timeSeries.nConfig, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "", 1, MAX_COLOR_VALUE + 2, "0", "", false, 1, 12, false);

    drawTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.agcControlValueH, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "AGC control", 1, MAX_COLOR_VALUE + 1, "0", "700", false, dotSize, 12, true);
    drawTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.agcControlValueV, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "", 1, 13, "", "", false, dotSize, 12, false);

    drawTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.x1H, nImagePairs, ox, oy + plotHeight + dy, plotWidth, plotHeight, dayStart, dayEnd, 25, 55, "", "x1", 1, MAX_COLOR_VALUE + 1, "25", "55", false, dotSize, 12, true);
    drawTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.x1V, nImagePairs, ox, oy + plotHeight + dy, plotWidth, plotHeight, dayStart, dayEnd, 25, 55, "", "", 1, 13, "", "", false, dotSize, 12, false);

    drawTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.y1H, nImagePairs, ox, oy + 2*plotHeight + 2*dy, plotWidth, plotHeight, dayStart, dayEnd, 20, 45, xlabel, "y1", 1, MAX_COLOR_VALUE + 1, "20", "45", false, dotSize, 12, true);
    drawTimeSeries(&image, imagePairTimeSeries.time, imagePairTimeSeries.y1V, nImagePairs, ox, oy + 2*plotHeight + 2*dy, plotWidth, plotHeight, dayStart, dayEnd, 20, 45, "", "", 1, 13, "", "", false, dotSize, 12, false);

    // AGC histograms
    ox = 640;
    oy = 220;
    double agcBinWidth = 50;
    double agcBinMin = 25;
    double agcBinMax = 800;
    int histWidth = 200;
    int histHeight = 150;
    // TODO Only include AGC enabled data?
    drawHistogram(&image, imagePairTimeSeries.agcControlValueH, imagePairTimeSeries.nImagePairs, agcBinWidth, agcBinMin, agcBinMax, histWidth, histHeight, ox, oy, HISTOGRAM_PEAK_EQUALS_ONE, "AGC control value H");

    drawHistogram(&image, imagePairTimeSeries.agcControlValueV, imagePairTimeSeries.nImagePairs, agcBinWidth, agcBinMin, agcBinMax, histWidth, histHeight, ox, oy + histHeight + 50, HISTOGRAM_PEAK_EQUALS_ONE, "AGC control value V");

    for (int c = 0; c < 3.0 * VIDEO_FPS; c++)
        generateFrame(&image, frameCounter++);


    finishVideo();

    // TODO
    // Get config packet info as needed.

    if (frameCounter > 0)
        printf("%s\n", movieFilename);
    else
        printf("No-Frames-For-This-Date\n");

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);

    freeLpTiiTimeSeries(&timeSeries);
    freeImagePairTimeSeries(&imagePairTimeSeries);

    freeImage(&templateImage);
    freeImage(&image);

    fflush(stdout);

    exit(EXIT_SUCCESS);
}

void usage(const char * name)
{
    printf("\nTII Movie Generator Version %s compiled %s %s UTC\n", TIIM_VERSION, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s SW_OPER_EFIXDDD_0__yyyyMMddThhmmss_yyyyMMddThhmmss_vvvv.HDR maxSignal outputDir [-f] \n", name);
    printf("\nor\n");
    printf("\n  %s Xyyyymmdd maxSignal outputDir [-f]\n", name);
    printf("\n");
    printf("In the first form DDD is either \"NOM\" or \"TIC\"\n");
    printf("In the second form X designates the Swarm satellite (A, B or C).\n");
    printf("Set maxSignal to -1 for autoscaling the TII imagery.\n");
    printf("\"-f\" forces overwriting an extant TII movie file.\n");

    return;
}
