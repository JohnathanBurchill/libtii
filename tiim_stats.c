#include "tiim.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"

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
    int imagesRead = 0;
    IspDateTime * dt = NULL;

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

    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS], pixelsV[NUM_FULL_IMAGE_PIXELS];
    uint8_t imageBuf[IMAGE_BUFFER_SIZE];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImagePair imagePair;
    ImageAuxData auxH, auxV;
    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    double maxValueH, maxValueV;

    ImageStats statsH, statsV;
    initializeImageStats(&statsH);
    initializeImageStats(&statsV);
    
    for (long i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);
        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        //analyze imagery
        analyzeImage(imagePair.pixelsH, imagePair.gotImageH, max, &statsH);
        analyzeImage(imagePair.pixelsV, imagePair.gotImageV, max, &statsV);


    }

    // Summary
    dt = getIspDateTime(&imagePair);
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

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    fflush(stdout);

    exit(0);
}

