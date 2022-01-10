#include "tiim.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include "draw.h"
#include "video.h"
#include "filters.h"

#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{
    if (argc < 4 || argc > 5)
    {
        usage(argv[0]);
        exit(1);
    }

    int status = 0;
    char movieFilename[FILENAME_MAX];
    int frameCounter = 0;
    int gotImagePairTimeSeries = 0;
    int gotLpTiiTimeSeries = 0;

    char * hdr = argv[1];
    size_t sourceLen = strlen(hdr);

    if (strcmp(hdr + sourceLen - 4, ".HDR") != 0 && sourceLen != 9)
    {
        usage(argv[0]);
	    exit(1);
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
    ImagePackets imagePackets;
    SciencePackets sciencePackets;

    LpTiiTimeSeries timeSeries;
    initLpTiiTimeSeries(&timeSeries);
    ImagePairTimeSeries imagePairTimeSeries;
    initImagePairTimeSeries(&imagePairTimeSeries);

    status = importImagery(hdr, &imagePackets);
    if (status)
    {
        fprintf(stderr, "Could not import image data.\n");
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
    uint8_t templateBuf[IMAGE_BUFFER_SIZE];
    uint8_t imageBuf[IMAGE_BUFFER_SIZE];
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
    drawTemplate(templateBuf, &timeSeries, &imagePairTimeSeries, dayStart, dayEnd);

    int nImagePairs = 0;

    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd))
            continue;

        memcpy(imageBuf, templateBuf, IMAGE_BUFFER_SIZE);
        drawFrame(imageBuf, templateBuf, &imagePair, &timeSeries, &imagePairTimeSeries, nImagePairs, frameCounter, dayStart, dayEnd);
        generateFrame(imageBuf, frameCounter);

        frameCounter++;
        nImagePairs++;


    }
    // Allow a moment before continuing with frame analysis
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    int nAcross = 16;
    int nDown = 8;
    int dx = 42;
    int dy = 67;
    int xborder = IMAGE_WIDTH / 2 - nAcross / 2 * dx;
    int yborder=3;

    // Frame-by-frame H
    int xoffset = 0;
    int yoffset = 0;
    insertTransition(imageBuf, "Frame-by-frame Horizontal sensor", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);

    nImagePairs = 0;
    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd))
            continue;

        if (xoffset == 0 && imagePair.gotImageH)
            drawTimestamp(imageBuf, 15, yborder + yoffset * dy + 25, imagePair.auxH, 12);
        if (xoffset == nAcross-1 && imagePair.gotImageH)
            drawTimestamp(imageBuf, IMAGE_WIDTH - 8*8-60, yborder + yoffset * dy+25, imagePair.auxH, 12);
            
        drawImage(imageBuf, imagePair.pixelsH, imagePair.gotImageH, imagePairTimeSeries.maxValueH[nImagePairs], xborder+(xoffset++)*dx, yborder+yoffset*dy, 1, false, imagePair.auxH, &identityFilter, NULL);
        xoffset %= nAcross;
        if (xoffset == 0) yoffset++;
        // Show for 3 seconds each
        if (xoffset == 0 && yoffset == nDown)
        {
            for (int c = 0; c < 3 * VIDEO_FPS; c++)
                generateFrame(imageBuf, frameCounter++);
            xoffset = 0;
            yoffset = 0;
            drawFill(imageBuf, BACKGROUND_COLOR);
        }
        nImagePairs++;
    }
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    // Frame-by-frame V
    xoffset = 0;
    yoffset = 0;
    insertTransition(imageBuf, "Frame-by-frame Vertical sensor", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);
    nImagePairs= 0;
    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd))
            continue;


        if (xoffset == 0 && imagePair.gotImageV)
            drawTimestamp(imageBuf, 15, yborder + yoffset * dy + 25, imagePair.auxV, 12);
        if (xoffset == nAcross-1 && imagePair.gotImageV)
            drawTimestamp(imageBuf, IMAGE_WIDTH - 8*8-60, yborder + yoffset * dy+25, imagePair.auxV, 12);

        drawImage(imageBuf, imagePair.pixelsV, imagePair.gotImageV, imagePairTimeSeries.maxValueV[nImagePairs], xborder+(xoffset++)*dx, yborder+yoffset*dy, 1, false, imagePair.auxV, &identityFilter, NULL);
        xoffset %= nAcross;
        if (xoffset == 0) yoffset++;
        // Show for 3 seconds each
        if (xoffset == 0 && yoffset == nDown)
        {
            for (int c = 0; c < 3 * VIDEO_FPS; c++)
                generateFrame(imageBuf, frameCounter++);
            xoffset = 0;
            yoffset = 0;
            drawFill(imageBuf, BACKGROUND_COLOR);
        }
        nImagePairs++;
    }
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    // Draw PA and measles time series
    int plotWidth = 500;
    int plotHeight = 100;
    int ox = 100;
    int oy = 140;
    int dotSize = 2;
    if (sourceLen == 9) dotSize = 2; // full day
    insertTransition(imageBuf, "Anomaly overview", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);
    drawIntTimeSeries(imageBuf, imagePairTimeSeries.time, imagePairTimeSeries.paCountH, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 1000, "", "PA Level", 1, MAX_COLOR_VALUE + 1, "0", "1000", false, dotSize, 12, true);
    drawIntTimeSeries(imageBuf, imagePairTimeSeries.time, imagePairTimeSeries.paCountV, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 1000, "", "", 1, 13, "", "", false, dotSize, 12, false);
    
    drawIntTimeSeries(imageBuf, imagePairTimeSeries.time, imagePairTimeSeries.measlesCountH, nImagePairs, ox, oy + plotHeight + 50, plotWidth, plotHeight, dayStart, dayEnd, 0, 200, "Hours from start of file", "Measles Level", 1, MAX_COLOR_VALUE + 1, "0", "200", false, dotSize, 12, true);
    drawIntTimeSeries(imageBuf, imagePairTimeSeries.time, imagePairTimeSeries.measlesCountV, nImagePairs, ox, oy + plotHeight + 50, plotWidth, plotHeight, dayStart, dayEnd, 0, 200, "", "", 1, 13, "", "", false, dotSize, 12, false);
    for (int c = 0; c < 3.0 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    // AGC review
    ox = 100;
    oy = 120;
    insertTransition(imageBuf, "AGC overview", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 3.0, &frameCounter);
    drawIntTimeSeries(imageBuf, timeSeries.configTime, timeSeries.agcLowerThresholdConfig, timeSeries.nConfig, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "", 1, MAX_COLOR_VALUE + 2, "0", "", false, 1, 12, false);
    drawIntTimeSeries(imageBuf, timeSeries.configTime, timeSeries.agcUpperThresholdConfig, timeSeries.nConfig, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "", 1, MAX_COLOR_VALUE + 2, "0", "", false, 1, 12, false);
    drawTimeSeries(imageBuf, imagePairTimeSeries.time, imagePairTimeSeries.agcControlValueH, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "AGC control", 1, MAX_COLOR_VALUE + 1, "0", "700", false, dotSize, 12, true);
    drawTimeSeries(imageBuf, imagePairTimeSeries.time, imagePairTimeSeries.agcControlValueV, nImagePairs, ox, oy, plotWidth, plotHeight, dayStart, dayEnd, 0, 700, "", "", 1, 13, "", "", false, dotSize, 12, false);
    for (int c = 0; c < 3.0 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);


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

    fflush(stdout);

    exit(0);
}

void usage(char * name)
{
    printf("\nTII Movie Generator Version %s compiled %s %s UTC\n", TIIM_VERSION, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2021 University of Calgary\n");
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
