#include "tiim.h"

#include "isp.h"
#include "xml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdint.h>
#include <spng.h>


int main(int argc, char **argv)
{
    printf("Hi there.\n");
    printf("So you want to read the Swarm TII image data...\n");
    if (argc != 2)
    {
        printf("usage: %s normalModeHeaderFile.HDR\n", argv[0]);
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

    size_t png_size;
    void *png_buf = NULL;
    int ret = 0;
    spng_ctx *enc = NULL;

    char * hdr = argv[1];
    size_t len = strlen(hdr);
    if (strcmp(hdr + len - 4, ".HDR") != 0)
    {
    	printf("We are sloppy today, aren't we? Try passing me a .HDR file.\n");
	    exit(1);
    }

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
    uint16_t pixels[NUM_FULL_IMAGE_PIXELS];
    char efiUnit[3] = {'C', 'B', 'A'};
//    for (int i = 0; i < numFullImageRecords; i++)
    for (int i = 0; i < 1; i++)
    {
        FullImagePacket *fip = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        FullImageContinuedPacket *cip = (FullImageContinuedPacket*)(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE);
        ImageAuxData aux;
        getImageData(fip, cip, &aux, pixels);
        printf("%4d:", i+1);
        printf(" %c %s", efiUnit[aux.EfiInstrumentId-1], aux.SensorNumber ? "V" : "H");
        printf(" (%5.1lf C), FP: %5.2lf V", aux.CcdTemperature, aux.FaceplateVoltageMonitor);
        printf(", ID: %6.1lf V", aux.BiasGridVoltageMonitor);
        printf(", MCP: %7.1lf V", aux.McpVoltageMonitor);
        printf(", PHOS: %7.1lf V", aux.PhosphorVoltageMonitor);
        printf("\n");
    }
    for (int i = 0; i < NUM_FULL_IMAGE_PIXELS; i++)
    {
        printf("%d ", pixels[i]);
    }
    printf("\n");

    // Make png files with text overlays
    enc = spng_ctx_new(SPNG_CTX_ENCODER);
    // int spng_set_png_file(spng_ctx *ctx, FILE *file);
    spng_set_option(enc, SPNG_ENCODE_TO_BUFFER, 1);
    struct spng_ihdr ihdr;
    ihdr.bit_depth = 16;
    ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE;
    ihdr.compression_method = SPNG_EICCP_COMPRESSION_METHOD;
    ihdr.height = 66;
    ihdr.width = 40;
    ihdr.interlace_method = SPNG_INTERLACE_NONE;
    ihdr.filter_method = SPNG_FILTER_NONE;
    //    spng_set_png_file(enc, pngFile); // Need to make a new file for each image
    spng_set_ihdr(enc, &ihdr);
    size_t out_size = 2640;
    unsigned char * out = NULL;
    out = malloc(out_size);
    if (spng_encode_image(enc, pixels, out_size, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE))
    {
        printf("spng_encode_image() error: %s\n", spng_strerror(ret));
//        goto cleanup;
    }
    png_buf = spng_get_png_buffer(enc, &png_size, &ret);
    if(png_buf == NULL)
    {
        printf("spng_get_png_buffer() error: %s\n", spng_strerror(ret));
    }

    for (int i = 0; i < png_size; i++)
    {
        printf("%d ", ((uint8_t*)png_buf)[i]);
    }
    printf("\n");
    // Get the ion admittance from LP&TII packets and convert to density
    // Get config packet info as needed.


cleanup:
    if (dblFile != NULL) fclose(dblFile);
    if (fullImagePackets != NULL) free(fullImagePackets);
    if (continuedPackets != NULL) free(continuedPackets);
    if (png_buf != NULL) free(png_buf);
    if (enc != NULL) spng_ctx_free(enc);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    exit(0);
}

