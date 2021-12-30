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

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("usage: %s normalModeHeaderFile.HDR maxSignal (pass -1 for autoscaling) outputDir\n", argv[0]);
        exit(1);
    }

    int status = 0;

    char * hdr = argv[1];
    size_t len = strlen(hdr);
    if (strcmp(hdr + len - 4, ".HDR") != 0)
    {
    	printf("usage: %s <normalModeHeaderFile>.HDR maxSignal outputDir\n", argv[0]);
	    exit(1);
    }

    double max = atof(argv[2]);

    char *outputDir = argv[3];

    // Data
    ImagePackets imagePackets;
    status = importImagery(hdr, &imagePackets);
    if (status)
    {
        fprintf(stderr, "Could not import image data.\n");
        goto cleanup;
    }
    if (imagePackets.numberOfImages == 0)
        goto cleanup;

    SciencePackets sciencePackets;
    // Continue even if we could not import science packets
    importScience(hdr, &sciencePackets);
    LpTiiTimeSeries timeSeries;
    getTimeSeries(&sciencePackets, &timeSeries);
    
    // printf("2Hz samples: %ld\n", timeSeries.n2Hz);
    // for (long i = 0; i < timeSeries.n2Hz; i+=1000)
    // {
    //     printf("time: %lf y2H: %.2lf y2V: %.2lf  Ni1: %lf  Ni2: %lf\n", timeSeries.lpTiiTime2Hz[i] - timeSeries.lpTiiTime2Hz[0], timeSeries.y2H[i], timeSeries.y2V[i], timeSeries.ionDensity1[i], timeSeries.ionDensity2[i]);
    // }

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

    char movieFilename[FILENAME_MAX];
    status = constructMovieFilename(&imagePackets, &imagePair, outputDir, movieFilename);
    if (status != UTIL_OK)
    {
        printf("Could not construct movie filename.\n");
        goto cleanup;
    }

    status = initVideo(movieFilename);
    if (status < 0)
    {
        fprintf(stderr, "Problem intializing video: got status %d.\n", status);
        goto cleanup;
    }

    // Construct frames and export MPEG
    int frameCounter = 0;

    ImageStats statsH, statsV;
    initializeImageStats(&statsH);
    initializeImageStats(&statsV);

    // Static content from frame to frame
    drawTemplate(templateBuf, &timeSeries);

    int nImagePairs = 0;

    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        nImagePairs++;

        //analyze imagery
        analyzeImage(imagePair.pixelsH, imagePair.gotImageH, max, &statsH);
        analyzeImage(imagePair.pixelsV, imagePair.gotImageV, max, &statsV);

        memcpy(imageBuf, templateBuf, IMAGE_BUFFER_SIZE);
        drawFrame(imageBuf, templateBuf, &imagePair, &statsH, &statsV, &timeSeries, frameCounter);
        generateFrame(imageBuf, frameCounter);
        frameCounter++;

    }

    int nAcross = 16;
    int nDown = 8;
    int dx = 42;
    int dy = 67;
    int xborder = IMAGE_WIDTH / 2 - nAcross / 2 * dx;
    int yborder=3;

    // Frame-by-frame H
    int xoffset = 0;
    int yoffset = 0;
    insertTransition(imageBuf, "Frame-by-frame Horizontal sensor", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 2.0, &frameCounter);

    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        analyzeImage(imagePair.pixelsH, imagePair.gotImageH, max, &statsH);

        drawImage(imageBuf, imagePair.pixelsH, imagePair.gotImageH, statsH.maxValue, xborder+(xoffset++)*dx, yborder+yoffset*dy, 1, &identityFilter, NULL);
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
    }
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    // Frame-by-frame V
    xoffset = 0;
    yoffset = 0;
    insertTransition(imageBuf, "Frame-by-frame Vertical sensor", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 2.0, &frameCounter);
    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        analyzeImage(imagePair.pixelsV, imagePair.gotImageV, max, &statsV);

        drawImage(imageBuf, imagePair.pixelsV, imagePair.gotImageV, statsV.maxValue, xborder+(xoffset++)*dx, yborder+yoffset*dy, 1, &identityFilter, NULL);
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
    }
    for (int c = 0; c < 3 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    // PA and Measles time series
    double *imageTimes = malloc((size_t)(nImagePairs*sizeof(double)));
    double *paCountH = malloc((size_t)(nImagePairs*sizeof(double)));
    double *paCountV = malloc((size_t)(nImagePairs*sizeof(double)));
    double *measlesCountH = malloc((size_t)(nImagePairs*sizeof(double)));
    double *measlesCountV = malloc((size_t)(nImagePairs*sizeof(double)));

    int imagePairCounter = 0;
    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        //analyze imagery
        analyzeImage(imagePair.pixelsH, imagePair.gotImageH, max, &statsH);
        analyzeImage(imagePair.pixelsV, imagePair.gotImageV, max, &statsV);
        imageTimes[imagePairCounter] = imagePair.secondsSince1970;
        paCountH[imagePairCounter] = (double)statsH.paCount;
        paCountV[imagePairCounter] = (double)statsV.paCount;
        measlesCountH[imagePairCounter] = (double)statsH.measlesCount;
        measlesCountV[imagePairCounter] = (double)statsV.measlesCount;
        imagePairCounter++;
    }
    // Draw PA and measles time series
    int plotWidth = 500;
    int plotHeight = 120;
    int ox = 200;
    int oy = 200;
    int dotSize = 3;
    insertTransition(imageBuf, "Anomaly overview", IMAGE_WIDTH/2, IMAGE_HEIGHT/2-16, 24, 2.0, &frameCounter);
    drawTimeSeries(imageBuf, imageTimes, paCountH, nImagePairs, ox, oy, plotWidth, plotHeight, imageTimes[0], imageTimes[nImagePairs-1], 0, 1000, "", "", 1, MAX_COLOR_VALUE + 1, "", "", false, dotSize, 12);
    drawTimeSeries(imageBuf, imageTimes, paCountV, nImagePairs, ox, oy, plotWidth, plotHeight, imageTimes[0], imageTimes[nImagePairs-1], 0, 1000, "", "PA Level", 1, 13, "0", "1000", false, dotSize, 12);
    
    drawTimeSeries(imageBuf, imageTimes, measlesCountH, nImagePairs, ox, oy + plotHeight + 50, plotWidth, plotHeight, imageTimes[0], imageTimes[nImagePairs-1], 0, 200, "", "", 1, MAX_COLOR_VALUE + 1, "", "", false, dotSize, 12);
    drawTimeSeries(imageBuf, imageTimes, measlesCountV, nImagePairs, ox, oy + plotHeight + 50, plotWidth, plotHeight, imageTimes[0], imageTimes[nImagePairs-1], 0, 200, "Hours from start of file", "Measles Level", 1, 13, "0", "200", false, dotSize, 12);
    for (int c = 0; c < 1.0 * VIDEO_FPS; c++)
        generateFrame(imageBuf, frameCounter++);

    finishVideo();

    // TODO
    // Get the ion admittance from LP&TII packets and convert to density
    // Get config packet info as needed.

    if (frameCounter > 0)
        printf("%s\n", movieFilename);
    else
        printf("No-Frames-For-This-Date\n");

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    fflush(stdout);

    exit(0);
}

