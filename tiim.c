#include "tiim.h"

#include "isp.h"
#include "xml.h"
#include "analysis.h"
#include "draw.h"
#include "png.h"
#include "colortable.h"
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
    xmlInitParser();
    LIBXML_TEST_VERSION

    FILE * dblFile = NULL;
    // memory pointers
    // FullImagePacket * fullImagePackets = NULL;
    uint8_t * fullImagePackets = NULL;
    uint8_t * continuedPackets = NULL;

    char * hdr = argv[1];
    size_t len = strlen(hdr);
    if (strcmp(hdr + len - 4, ".HDR") != 0)
    {
    	printf("We are sloppy today, aren't we? Try passing me a .HDR file.\n");
	    exit(1);
    }

    double max = atof(argv[2]);

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    if ((doc = xmlReadFile(hdr, NULL, 0)) == NULL)
    {
        printf("Nothing to see. Try pointing me to a real file that I am allowed to read.\n");
        exit(1);
    }

    HdrInfo fi, ci;
    status = parseHdr(doc, &fi, &ci);
    if (status)
    {
        printf("Could not parse HDR file.\n");
        exit(status);
    }

    // get DBL filename and check that we can open it.
    char dbl[FILENAME_MAX];
    snprintf(dbl, strlen(hdr)-3, "%s", hdr);
    sprintf(dbl, "%s.DBL", dbl);

    char pngFile[FILENAME_MAX];

    if (access(dbl, R_OK))
    {
        printf("I need a readable .DBL file in the same folder as the .HDR file. Game over, try again.\n");
        goto cleanup;
    }

    dblFile = fopen(dbl, "r");
    if (dblFile == NULL)
    {
        printf("I'm having trouble reading your .DBL file.\n");
        goto cleanup;
    }
    size_t fullImageTotalBytes = (size_t) fi.numRecords * (size_t) fi.recordSize;
    fullImagePackets = (uint8_t*) malloc(fullImageTotalBytes * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        printf("Danger, Will Robinson! I could not find memory to store the full image packets.");
        goto cleanup;
    }
    size_t continuedTotalBytes = (size_t) ci.numRecords * (size_t) ci.recordSize;
    continuedPackets = (uint8_t*) malloc(continuedTotalBytes * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        printf("Danger, Will Robinson! I could not find memory to store the full image continued packets.");
        goto cleanup;
    }


    size_t bytesRead = 0;
    // Set file offset to read full image packets
    if (fseek(dblFile, fi.offset, SEEK_SET))
    {
        printf("Your OS is giving me a hard time - I was't able to seek to the full-image packets in the .DBL file.\n");
        printf("It tells me this: %s\n", strerror(errno));
    }
    if ((bytesRead = fread((uint8_t*)fullImagePackets, sizeof(uint8_t), fullImageTotalBytes, dblFile)) != fullImageTotalBytes)
    {
        printf("Had trouble reading the full image bytes. Expected to read %ld bytes, got %ld bytes. Bye.\n", fullImageTotalBytes, bytesRead);
        goto cleanup;
    }
    // Set file offset to read full image continued packets
    if (fseek(dblFile, ci.offset, SEEK_SET))
    {
        printf("Your OS is giving me a hard time - I was't able to seek to the full-image-continued packets in the .DBL file.\n");
        printf("It tells me this: %s\n", strerror(errno));
    }
    if ((bytesRead = fread((uint8_t*)continuedPackets, sizeof(uint8_t), continuedTotalBytes, dblFile)) != continuedTotalBytes)
    {
        printf("Had trouble reading the full-image-continued bytes. Expected to read %ld bytes, got %ld bytes. Bye.\n", continuedTotalBytes, bytesRead);
        goto cleanup;
    }
    // Set file offset to read full image continued packets
    uint16_t pixels1[NUM_FULL_IMAGE_PIXELS], pixels2[NUM_FULL_IMAGE_PIXELS];
    uint8_t imageBuf[IMAGE_BUFFER_SIZE];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImageAuxData aux1, aux2;
    double maxValueH, maxValueV;

    // anomaly statistics
    int paCountsH, paCountsV, cumulativePaCountsH, cumulativePaCountsV;
    int measlesCountsH, measlesCountsV, cumulativeMeaslesCountsH, cumulativeMeaslesCountsV;


    struct spng_plte colorTable;
    colorTable.n_entries = 256;
    for (int c = MIN_COLOR_VALUE; c <= MAX_COLOR_VALUE; c++)
    {
        colorTable.entries[c].red = colorsrgbrgb[3*c];
        colorTable.entries[c].green = colorsrgbrgb[3*c + 1];
        colorTable.entries[c].blue = colorsrgbrgb[3*c + 2];
    }
    // Foreground black
    colorTable.entries[252].red = FONTLEVEL1;
    colorTable.entries[252].green = FONTLEVEL1;
    colorTable.entries[252].blue = FONTLEVEL1;
    colorTable.entries[253].red = FONTLEVEL2;
    colorTable.entries[253].green = FONTLEVEL2;
    colorTable.entries[253].blue = FONTLEVEL2;
    colorTable.entries[254].red = FONTLEVEL3;
    colorTable.entries[254].green = FONTLEVEL3;
    colorTable.entries[254].blue = FONTLEVEL3;
    // Background white / transparent
    colorTable.entries[BACKGROUND_COLOR].red = 255;
    colorTable.entries[BACKGROUND_COLOR].green = 255;
    colorTable.entries[BACKGROUND_COLOR].blue = 255;
    colorTable.entries[BACKGROUND_COLOR].alpha = 0;


    // Get start and stop times
    getImagePair(fullImagePackets, continuedPackets, 0, fi.numRecords, &aux1, pixels1, &aux2, pixels2);
    char startDate[16];
    memset(startDate, 0, 16);
    sprintf(startDate, "%04d%02d%02dT%02d%02d%02d", aux1.year, aux1.month, aux1.day, aux1.hour, aux1.minute, aux1.second);
    getImagePair(fullImagePackets, continuedPackets, fi.numRecords-2, fi.numRecords, &aux1, pixels1, &aux2, pixels2);
    char stopDate[16];
    memset(stopDate, 0, 16);
    sprintf(stopDate, "%04d%02d%02dT%02d%02d%02d", aux2.year, aux2.month, aux2.day, aux2.hour, aux2.minute, aux2.second);
    char movieFilename[60];
    sprintf(movieFilename, "SW_OPER_EFI%cTIIMOV_%s_%s_%s.mp4", aux1.satellite, startDate, stopDate, TIIM_VERSION);

    // Construct frames and export to PNG files
    int filenameCounter = 0;
    bool gotHImage;
    bool gotVImage;

    ImageStats statsH;
    statsH.cumulativePaCount = 0;
    statsH.cumulativeMeaslesCount = 0;
    ImageStats statsV;
    statsV.cumulativePaCount = 0;
    statsV.cumulativeMeaslesCount = 0;

    for (int i = 0; i < fi.numRecords-1; i+=2)
    // for (int i = 0; i < 2-1; i+=2)
    {
        getImagePair(fullImagePackets, continuedPackets, i, fi.numRecords, &aux1, pixels1, &aux2, pixels2);
        alignImages(&aux1, pixels1, &aux2, pixels2, &i, &gotHImage, &gotVImage, &(fi.numRecords));
        sprintf(pngFile, "EFI%c_%05d.png", gotHImage ? aux1.satellite : aux2.satellite, filenameCounter);

        //analyze imagery
        analyzeImage(pixels1, gotHImage, max, &statsH);
        analyzeImage(pixels2, gotVImage, max, &statsV);

        // Prepare the PNG frame buffer
        drawImage(imageBuf, &aux1, pixels1, gotHImage, &statsH, &aux2, pixels2, gotVImage, &statsV);

        // Write the frame to file
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
    if (dblFile != NULL) fclose(dblFile);
    if (fullImagePackets != NULL) free(fullImagePackets);
    if (continuedPackets != NULL) free(continuedPackets);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    fflush(stdout);

    exit(0);
}

