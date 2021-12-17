#include "tiim.h"

#include "isp.h"
#include "xml.h"
#include "import.h"
#include "analysis.h"
#include "draw.h"
#include "png.h"
#include "fonts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdint.h>
#include <spng.h>
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
    	printf("usage: %s normalModeHeaderFile.HDR maxSignal.\n", argv[0]);
	    exit(1);
    }

    double max = atof(argv[2]);

    HdrInfo fi, ci;
    status = parseHdr(hdr, &fi, &ci);
    if (status)
    {
        printf("Could not parse HDR file.\n");
        exit(status);
    }

    // memory pointers
    uint8_t * fullImagePackets = NULL;
    uint8_t * continuedPackets = NULL;

    long nImages = 0;

    status = importImagery(hdr, &fi, &ci, &fullImagePackets, &continuedPackets, &nImages);
    if (status)
    {
        printf("Could not import image data.\n");
        goto cleanup;
    }


    char pngFile[FILENAME_MAX];
    uint16_t pixels1[NUM_FULL_IMAGE_PIXELS], pixels2[NUM_FULL_IMAGE_PIXELS];
    uint8_t imageBuf[IMAGE_BUFFER_SIZE];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImageAuxData aux1, aux2;
    double maxValueH, maxValueV;
    int imagesRead = 0;
    bool gotHImage;
    bool gotVImage;

    // Get time of first valid image
    // Need valid UNIT ID so loop forward until we get a valid image
    char startDate[16];
    memset(startDate, 0, 16);
    char stopDate[16];
    memset(stopDate, 0, 16);
    int packetIndex = 0;
    char satellite = 'X';
    char movieFilename[FILENAME_MAX];
    while((status = getAlignedImagePair(fullImagePackets, continuedPackets, packetIndex++, nImages, &aux1, pixels1, &aux2, pixels2, &gotHImage, &gotVImage, &imagesRead)) == ISP_NO_IMAGE_PAIR && packetIndex < nImages);
    switch(status)
    {
        case ISP_ALIGNED_IMAGE_PAIR:
        case ISP_H_IMAGE:
            sprintf(startDate, "%04d%02d%02dT%02d%02d%02d", aux1.dateTime.year, aux1.dateTime.month, aux1.dateTime.day, aux1.dateTime.hour, aux1.dateTime.minute, aux1.dateTime.second);
            satellite = aux1.satellite;
            break;
        case ISP_V_IMAGE:
            sprintf(startDate, "%04d%02d%02dT%02d%02d%02d", aux2.dateTime.year, aux2.dateTime.month, aux2.dateTime.day, aux2.dateTime.hour, aux2.dateTime.minute, aux2.dateTime.second);
            satellite = aux2.satellite;
            break;
        default:
            printf("Could not find valid start time.\n");
            goto cleanup;
    }
    packetIndex = nImages-1;
    while((status = getAlignedImagePair(fullImagePackets, continuedPackets, packetIndex--, nImages, &aux1, pixels1, &aux2, pixels2, &gotHImage, &gotVImage, &imagesRead)) == ISP_NO_IMAGE_PAIR && packetIndex >= 0);
    switch(status)
    {
        case ISP_ALIGNED_IMAGE_PAIR:
        case ISP_H_IMAGE:
            sprintf(stopDate, "%04d%02d%02dT%02d%02d%02d", aux1.dateTime.year, aux1.dateTime.month, aux1.dateTime.day, aux1.dateTime.hour, aux1.dateTime.minute, aux1.dateTime.second);
            satellite = aux1.satellite;
            break;
        case ISP_V_IMAGE:
            sprintf(stopDate, "%04d%02d%02dT%02d%02d%02d", aux2.dateTime.year, aux2.dateTime.month, aux2.dateTime.day, aux2.dateTime.hour, aux2.dateTime.minute, aux2.dateTime.second);
            satellite = aux2.satellite;
            break;
        default:
            printf("Could not find valid stop time.\n");
            goto cleanup;
    }
    sprintf(movieFilename, "SW_OPER_EFI%cTIIMOV_%s_%s_%s.mp4", satellite, startDate, stopDate, TIIM_VERSION);

    // Construct frames and export to PNG files
    int filenameCounter = 0;

    ImageStats statsH;
    statsH.cumulativePaCount = 0;
    statsH.cumulativeMeaslesCount = 0;
    ImageStats statsV;
    statsV.cumulativePaCount = 0;
    statsV.cumulativeMeaslesCount = 0;

    struct spng_plte colorTable = getColorTable();

    for (long i = 0; i < nImages-1;)
    {
        status = getAlignedImagePair(fullImagePackets, continuedPackets, i, nImages, &aux1, pixels1, &aux2, pixels2, &gotHImage, &gotVImage, &imagesRead);
        i+=imagesRead;
        if (status == ISP_NO_IMAGE_PAIR)
            continue;

        //analyze imagery
        analyzeImage(pixels1, gotHImage, max, &statsH);
        analyzeImage(pixels2, gotVImage, max, &statsV);

        // Prepare the PNG frame buffer
        drawImage(imageBuf, &aux1, pixels1, gotHImage, &statsH, &aux2, pixels2, gotVImage, &statsV);

        // Write the frame to file
        sprintf(pngFile, "EFI%c_%05d.png", gotHImage ? aux1.satellite : aux2.satellite, filenameCounter);
        if (!writePng(pngFile, imageBuf, IMAGE_WIDTH, IMAGE_HEIGHT, &colorTable))
        {
            filenameCounter++;
        }


    }

    // TODO
    // Get the ion admittance from LP&TII packets and convert to density
    // Get config packet info as needed.

    if (filenameCounter > 0)
        printf("%s\n", movieFilename);
    else
        printf("No-Frames-For-This-Date\n");


cleanup:
    if (fullImagePackets != NULL) free(fullImagePackets);
    if (continuedPackets != NULL) free(continuedPackets);
    fflush(stdout);

    exit(0);
}

