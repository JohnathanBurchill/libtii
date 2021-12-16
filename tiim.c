#include "tiim.h"

#include "isp.h"
#include "xml.h"
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
    printf("Hi there.\n");
    printf("So you want to read the Swarm TII image data...\n");
    if (argc != 3)
    {
        printf("usage: %s normalModeHeaderFile.HDR maxSignal (pass -1 for autoscaling)\n", argv[0]);
        exit(1);
    }
    printf("Okay, let's have a look at the HDR file. Hope you passed that as your argument.\n");

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


    printf("What do we have here?\n");

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    if ((doc = xmlReadFile(hdr, NULL, 0)) == NULL)
    {
        printf("Nothing to see. Try pointing me to a real file that I am allowed to read.\n");
        exit(1);
    }
    long fullImageOffset = 0;
    long numFullImageRecords = 0;
    long fullImageBytes = 0;
    long continuedImageOffset = 0;
    long numContinuedImageRecords = 0;
    long continuedImageBytes = 0;
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Data_Set_Offset", &fullImageOffset))
    {
        printf("Full image offset not found. Bye.\n");
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Num_of_Records", &numFullImageRecords))
    {
        printf("Full image number of records not found. Bye.\n");
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Record_Size", &fullImageBytes))
    {
        printf("Full image record size not found. Bye.\n");
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Data_Set_Offset", &continuedImageOffset))
    {
        printf("Full image continued offset not found. Bye.\n");
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Num_of_Records", &numContinuedImageRecords))
    {
        printf("Full image continued number of records not found. Bye.\n");
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Record_Size", &continuedImageBytes))
    {
        printf("Full image continued record size not found. Bye.\n");
        goto cleanup;
    }

    if (fullImageBytes != FULL_IMAGE_PACKET_SIZE || continuedImageBytes != FULL_IMAGE_CONT_PACKET_SIZE)
    {
        printf("Something's amiss: one of the packet sizes is not what I expected. Might want to check the .HDR file.\n");
        goto cleanup;
    }
    if (numFullImageRecords != numContinuedImageRecords)
    {
        printf("Incomplete images found. Skipping this file.\n");
        goto cleanup;
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
    size_t fullImageTotalBytes = (size_t) numFullImageRecords * (size_t) fullImageBytes;
    fullImagePackets = (uint8_t*) malloc(fullImageTotalBytes * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        printf("Danger, Will Robinson! I could not find memory to store the full image packets.");
        goto cleanup;
    }
    size_t continuedTotalBytes = (size_t) numContinuedImageRecords * (size_t) continuedImageBytes;
    continuedPackets = (uint8_t*) malloc(continuedTotalBytes * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        printf("Danger, Will Robinson! I could not find memory to store the full image continued packets.");
        goto cleanup;
    }


    size_t bytesRead = 0;
    // Set file offset to read full image packets
    if (fseek(dblFile, fullImageOffset, SEEK_SET))
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
    if (fseek(dblFile, continuedImageOffset, SEEK_SET))
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
    int x, y;
    double maxValueH, maxValueV;
    int imageIndex;
    int p = 0;

    struct spng_plte colorTable;
    uint8_t background = 255; // white / transparent
    int maxLevel = 253; // 254 is black, 255 is white (and background)
    int minLevel = 0;
    colorTable.n_entries = 256;
    for (int c = minLevel; c <= maxLevel; c++)
    {
        colorTable.entries[c].red = colorsrgbrgb[3*c];
        colorTable.entries[c].green = colorsrgbrgb[3*c + 1];
        colorTable.entries[c].blue = colorsrgbrgb[3*c + 2];
    }
    // Foreground black
    colorTable.entries[254].red = 0;
    colorTable.entries[254].green = 0;
    colorTable.entries[254].blue = 0;
    // Background white / transparent
    colorTable.entries[255].red = 255;
    colorTable.entries[255].green = 255;
    colorTable.entries[255].blue = 255;
    colorTable.entries[255].alpha = 0;

    char months[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int lineSpacing = 16;

    for (int i = 0; i < numFullImageRecords-1; i+=2)
    // for (int i = 0; i < 2-1; i+=2)
    {
        // TODO pair images with H on left, V on right.
        fip1 = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        cip1 = (FullImageContinuedPacket*)(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE);
        fip2 = (FullImagePacket*)(fullImagePackets + (i+1)*FULL_IMAGE_PACKET_SIZE);
        cip2 = (FullImageContinuedPacket*)(continuedPackets + (i+1)*FULL_IMAGE_CONT_PACKET_SIZE);
        getImageData(fip1, cip1, &aux1, pixels1);
        getImageData(fip2, cip2, &aux2, pixels2);
        // printf("%4d:", i+1);
        // printf(" %c %s", efiUnit[aux.EfiInstrumentId-1], aux1.SensorNumber ? "V" : "H");
        // printf(" (%5.1lf C), FP: %5.2lf V", aux1.CcdTemperature, aux1.FaceplateVoltageMonitor);
        // printf(", ID: %6.1lf V", aux1.BiasGridVoltageMonitor);
        // printf(", MCP: %7.1lf V", aux1.McpVoltageMonitor);
        // printf(", PHOS: %7.1lf V", aux1.PhosphorVoltageMonitor);
        // printf("\n");
        sprintf(pngFile, "EFI%c_%05d.png", aux1.satellite, i);
        p = 0;
        double v;
        memset(imageBuf, background, IMAGE_BUFFER_SIZE);
        if (max < 0.0)
        {
            maxValueH = -1.0;
            maxValueV = -1.0;
            for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++)
            {
                if ((double)pixels1[k] > maxValueH)
                    maxValueH = pixels1[k];
                if ((double)pixels2[k] > maxValueV)
                    maxValueV = pixels2[k];
            }
        }
        else
        {
            maxValueH = max;
            maxValueV = max;
        }
        for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
        {

            v = floor((double)pixels1[k] / maxValueH * maxLevel);
            if (v > maxLevel) v = maxLevel;
            if (v < minLevel) v = minLevel;
            x = k / 66;
            y = 65 - (k % 66);
            for (int sx = 0; sx < IMAGE_SCALE; sx++)
            {
                for (int sy = 0; sy < IMAGE_SCALE; sy++)
                {
                    imageIndex = (IMAGE_WIDTH*(IMAGE_SCALE*(y)+sy+IMAGE_OFFSET_Y)+(IMAGE_SCALE*(x)+sx+IMAGE_OFFSET_X));
                    if (imageIndex > IMAGE_BUFFER_SIZE-1) imageIndex = IMAGE_BUFFER_SIZE -1;
                    imageBuf[imageIndex] = v;
                }
            }
            p++;
        }
        for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++ )
        {
            v = floor((double)pixels2[k] / maxValueV * maxLevel);
            if (v > maxLevel) v = maxLevel;
            if (v < minLevel) v = minLevel;
            x = k / 66;
            y = 65 - (k % 66);
            for (int sx = 0; sx < IMAGE_SCALE; sx++)
            {
                for (int sy = 0; sy < IMAGE_SCALE; sy++)
                {
                    imageIndex = (IMAGE_WIDTH*(IMAGE_SCALE*(y)+sy+IMAGE_OFFSET_Y)+(IMAGE_SCALE*(x)+sx + IMAGE_SCALE*V_IMAGE_OFFSET_X + IMAGE_OFFSET_X));
                    if (imageIndex > IMAGE_BUFFER_SIZE-1) imageIndex = IMAGE_BUFFER_SIZE -1;
                    imageBuf[imageIndex] = v;
                }
            }
            p++;
        }

        // color scales
        int xoff = IMAGE_OFFSET_X + 270;
        int yoff = IMAGE_OFFSET_Y + IMAGE_SCALE * 66 - 37;
        int cbarWidth = 8;
        int cbarSeparation = 30;
        for (int x = xoff; x < xoff+cbarWidth; x++)
        {
            for (int y = minLevel; y <= maxLevel; y+=2)
            {
                imageBuf[IMAGE_WIDTH * (yoff - y/2) + x] = y;
                imageBuf[IMAGE_WIDTH * (yoff - y/2) + x + cbarWidth + cbarSeparation] = y;
            }
        }
        char title[40];
        memset(title, 0, 40);

        annotate("DN", 12, xoff+cbarWidth + cbarSeparation/2 - 7, yoff + 7, imageBuf);
        annotate("H", 12, xoff, yoff - maxLevel / 2 - 20, imageBuf);
        annotate("V", 12, xoff + cbarWidth + cbarSeparation, yoff - maxLevel / 2 - 20, imageBuf);
        annotate("0", 9, xoff+cbarWidth+3, yoff-10, imageBuf);
        annotate("0", 9, xoff + cbarWidth + cbarSeparation + cbarWidth+3, yoff-10, imageBuf);
        sprintf(title, "%d", (int)floor(maxValueH));
        annotate(title, 9, xoff+cbarWidth+3, yoff - maxLevel/2+3, imageBuf);
        sprintf(title, "%d", (int)floor(maxValueV));
        annotate(title, 9, xoff + cbarWidth + cbarSeparation + cbarWidth+3, yoff - maxLevel/2+3, imageBuf);

        // Aux data
        int mo = (aux1.month-1)*3;
        sprintf(title, "Swarm %c %2d %c%c%c %4d %02d:%02d:%02d UT", aux1.satellite, aux1.day, months[mo], months[mo+1], months[mo+2], aux1.year, aux1.hour, aux1.minute, aux1.second);
        annotate(title, 15, IMAGE_WIDTH/2 - strlen(title)/2*10, 5, imageBuf);

        annotate("H", 15, 85, IMAGE_OFFSET_Y + 200, imageBuf);
        annotate("V", 15, 220, IMAGE_OFFSET_Y + 200, imageBuf);

        // Add times in images for montages
        sprintf(title, "%c %02d:%02d:%02d UT", aux1.sensor, aux1.hour, aux1.minute, aux1.second);
        annotate(title, 9, 30, 40, imageBuf);
        sprintf(title, "%c %02d:%02d:%02d UT", aux2.sensor, aux2.hour, aux2.minute, aux2.second);
        annotate(title, 9, 165, 40, imageBuf);

        // Monitors
        int monYOff = 60;
        annotate("H", 15, 490, 30 + monYOff, imageBuf);
        annotate("V", 15, 490 + 70, 30 + monYOff, imageBuf);

        sprintf(title, "      MCP: %6.0lf V", aux1.McpVoltageMonitor);
        annotate(title, 12, 370, 50 + monYOff, imageBuf);
        sprintf(title, "     Phos: %6.0lf V", aux1.PhosphorVoltageMonitor);
        annotate(title, 12, 370, 50 + lineSpacing + monYOff, imageBuf);
        sprintf(title, "  ID Bias: %6.1lf V", aux1.BiasGridVoltageMonitor);
        annotate(title, 12, 370, 50 + 2 * lineSpacing + monYOff, imageBuf);
        sprintf(title, "Faceplate: %6.1lf V", aux1.FaceplateVoltageMonitor);
        annotate(title, 12, 370, 50 + 3 * lineSpacing + monYOff, imageBuf);
        sprintf(title, "CCD temp.: %6.1lf C", aux1.CcdTemperature);
        annotate(title, 12, 370, 50 + 4 * lineSpacing + monYOff, imageBuf);

        sprintf(title, "  %6.0lf V", aux2.McpVoltageMonitor);
        annotate(title, 12, 370 + 150, 50 + monYOff, imageBuf);
        sprintf(title, "  %6.0lf V", aux2.PhosphorVoltageMonitor);
        annotate(title, 12, 370 + 150, 50 + lineSpacing + monYOff, imageBuf);
        sprintf(title, "  %6.1lf V", aux2.BiasGridVoltageMonitor);
        annotate(title, 12, 370 + 150, 50 + 2 * lineSpacing + monYOff, imageBuf);
        sprintf(title, "  %6.1lf V", aux2.FaceplateVoltageMonitor);
        annotate(title, 12, 370 + 150, 50 + 3 * lineSpacing + monYOff, imageBuf);
        sprintf(title, "  %6.1lf C", aux2.CcdTemperature);
        annotate(title, 12, 370 + 150, 50 + 4 * lineSpacing + monYOff, imageBuf);

        if (writePng(pngFile, imageBuf, IMAGE_WIDTH, IMAGE_HEIGHT, &colorTable))
        {
            printf("I couldn't write the PNG file. Sorry.\n");
            goto cleanup;
        }

    }
    // for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
    // {
    //     printf("%d ", pixels[i]);
    // }
    // printf("\n");

    // Get the ion admittance from LP&TII packets and convert to density
    // Get config packet info as needed.


cleanup:
    if (dblFile != NULL) fclose(dblFile);
    if (fullImagePackets != NULL) free(fullImagePackets);
    if (continuedPackets != NULL) free(continuedPackets);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    exit(0);
}

