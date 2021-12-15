#include "tiim.h"

#include "isp.h"
#include "xml.h"
#include "png.h"

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
    char efiUnit[3] = {'C', 'B', 'A'};
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImageAuxData aux1, aux2;
    int x, y;
    double maxValueH, maxValueV;
    int imageIndex;
    int p = 0;
    // for (int i = 0; i < numFullImageRecords-1; i+=2)
    for (int i = 0; i < 2-1; i+=2)
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
        sprintf(pngFile, "EFI%c_%05d.png", efiUnit[aux1.EfiInstrumentId-1], i);
        p = 0;
        double v;
        memset(imageBuf, 255, IMAGE_BUFFER_SIZE);
        if (max < 0.0)
        {
            maxValueH = -1.0;
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

            v = floor((double)pixels1[k] / maxValueH * 255.);
            if (v > 255) v = 255;
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
            v = floor((double)pixels2[k] / maxValueV * 255.);
            if (v > 255) v = 255;
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
        if (writePng(pngFile, imageBuf, IMAGE_WIDTH, IMAGE_HEIGHT))
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

