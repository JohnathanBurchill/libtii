#include "tiim.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include "draw.h"
#include "video.h"

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

    // Construct frames and export MPEG format
    int frameCounter = 0;

    ImageStats statsH, statsV;
    initializeImageStats(&statsH);
    initializeImageStats(&statsV);

    for (int i = 0; i < imagePackets.numberOfImages-1;)
    {
        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);

        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        //analyze imagery
        analyzeImage(imagePair.pixelsH, imagePair.gotImageH, max, &statsH);
        analyzeImage(imagePair.pixelsV, imagePair.gotImageV, max, &statsV);

        drawFrame(imageBuf, &imagePair, &statsH, &statsV, &timeSeries, frameCounter);
        generateFrame(imageBuf, frameCounter);
        frameCounter++;

    }

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

