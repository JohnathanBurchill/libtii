#include "tiim.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"

#ifndef ANALYSIS_ONLY
#include "draw.h"
#include "png.h"
#include "spng.h"
#include "fonts.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: %s normalModeHeaderFile.HDR maxSignal (pass -1 for autoscaling)\n", argv[0]);
        exit(1);
    }

    int status = 0;

    char * hdr = argv[1];
    size_t len = strlen(hdr);
    if (strcmp(hdr + len - 4, ".HDR") != 0)
    {
    	printf("usage: %s <normalModeHeaderFile>.HDR maxSignal.\n", argv[0]);
	    exit(1);
    }

    double max = atof(argv[2]);

    // Data
    ImagePackets imagePackets;
    imagePackets.fullImagePackets = NULL;
    imagePackets.continuedPackets = NULL;
    imagePackets.numberOfImages = 0;
    status = importImagery(hdr, &imagePackets);
    if (status)
    {
        fprintf(stderr, "Could not import image data.\n");
        goto cleanup;
    }
    if (imagePackets.numberOfImages == 0)
    {
        goto cleanup;
    }

    char pngFile[FILENAME_MAX];
    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS], pixelsV[NUM_FULL_IMAGE_PIXELS];
    uint8_t imageBuf[IMAGE_BUFFER_SIZE];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImagePair imagePair;
    ImageAuxData auxH, auxV;
    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    double maxValueH, maxValueV;
    int imagesRead = 0;

#ifndef ANALYSIS_ONLY
    char movieFilename[FILENAME_MAX];
    status = constructMovieFilename(&imagePackets, &imagePair, movieFilename);
    if (status != UTIL_OK)
    {
        printf("Could not construct movie filename.\n");
        goto cleanup;
    }
#endif

    // Construct frames and export to PNG files
    int filenameCounter = 0;

    ImageStats statsH;
    statsH.cumulativePaCount = 0;
    statsH.cumulativeMeaslesCount = 0;
    statsH.paCumulativeFrameCount = 0;
    statsH.maxPaValue =0;
    ImageStats statsV;
    statsV.cumulativePaCount = 0;
    statsV.cumulativeMeaslesCount = 0;
    statsV.paCumulativeFrameCount = 0;
    statsV.maxPaValue =0;
    for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
    {
        statsH.paAngularSpectrumCumulativeFrameCount[b] = 0;
        statsH.paAngularSpectrumTotal[b] = 0;
        statsV.paAngularSpectrumCumulativeFrameCount[b] = 0;
        statsV.paAngularSpectrumTotal[b] = 0;
    }

#ifndef ANALYSIS_ONLY
    struct spng_plte colorTable = getColorTable();
#endif

    for (long i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);
        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        //analyze imagery
        analyzeImage(imagePair.pixelsH, imagePair.gotImageH, max, &statsH);
        analyzeImage(imagePair.pixelsV, imagePair.gotImageV, max, &statsV);

#ifndef ANALYSIS_ONLY
        drawFrame(imageBuf, &imagePair, &statsH, &statsV);
        // Write the frame to file
        sprintf(pngFile, "EFI%c_%05d.png", getSatellite(&imagePair), filenameCounter);
        if (!writePng(pngFile, imageBuf, IMAGE_WIDTH, IMAGE_HEIGHT, &colorTable))
        {
            filenameCounter++;
        }
#endif

    }

    // TODO
    // Get the ion admittance from LP&TII packets and convert to density
    // Get config packet info as needed.

    // Summary

#ifdef ANALYSIS_ONLY
    IspDateTime * dt = getIspDateTime(&imagePair);
    // time (sec from 1970), satLetter, imagePairs, measlesCountH, measlesCountV, paCumulativeFrameCountH, paCumulativeFrameCountV, paAngularFrameCountsH... paAngularFramecountsV...
    printf("%ld %c %ld %d %d %d %d", (time_t)floor(dt->secondsSince1970), getSatellite(&imagePair), imagePackets.numberOfImages, statsH.cumulativeMeaslesCount, statsV.cumulativeMeaslesCount, statsH.paCumulativeFrameCount, statsV.paCumulativeFrameCount);
    printf(" %d", PA_ANGULAR_NUM_BINS);
    for (int i = 0; i < PA_ANGULAR_NUM_BINS; i++)
    {
        printf(" %d", statsH.paAngularSpectrumCumulativeFrameCount[i]);
    }
    for (int i = 0; i < PA_ANGULAR_NUM_BINS; i++)
    {
        printf(" %d", statsV.paAngularSpectrumCumulativeFrameCount[i]);
    }
    printf("\n");
#else

    if (filenameCounter > 0)
        printf("%s\n", movieFilename);
    else
        printf("No-Frames-For-This-Date\n");

#endif

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    fflush(stdout);

    exit(0);
}

