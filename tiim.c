#include "tiim.h"

#include "isp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdint.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

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


    char * hdr = argv[1];
    size_t len = strlen(hdr);
    if (strcmp(hdr + len - 4, ".HDR") != 0)
    {
	printf("My, my! We are absent-minded today, aren't we?\n");
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

    printf("Full image          : offset = %ld, # records = %ld, record size = %ld\n", fullImageOffset, numFullImageRecords, fullImageBytes);
    printf("Full image continued: offset = %ld, # records = %ld, record size = %ld\n", continuedImageOffset, numContinuedImageRecords, continuedImageBytes);

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
    printf("%s\n", dbl);

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
        printf("I could not find memory to store the full image packets.");
        goto cleanup;
    }
    size_t continuedTotalBytes = (size_t) numContinuedImageRecords * (size_t) continuedImageBytes;
    continuedPackets = (uint8_t*) malloc(continuedTotalBytes * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        printf("I could not find memory to store the full image continued packets.");
        goto cleanup;
    }


    size_t bytesRead = 0;
    // Set file offset to read full image packets
    if (fseek(dblFile, fullImageOffset, SEEK_SET))
    {
        printf("Your OS is giving me a hard time - I wan't able to seek the full image packets in the .DBL file.\n");
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
        printf("Your OS is giving me a hard time - I wan't able to seek the full image packets continued in the .DBL file.\n");
        printf("It tells me this: %s\n", strerror(errno));
    }
    if ((bytesRead = fread((uint8_t*)continuedPackets, sizeof(uint8_t), continuedTotalBytes, dblFile)) != continuedTotalBytes)
    {
        printf("Had trouble reading the full image continued bytes. Expected to read %ld bytes, got %ld bytes. Bye.\n", continuedTotalBytes, bytesRead);
        goto cleanup;
    }

    // Set file offset to read full image continued packets
    uint16_t pixels[NUM_FULL_IMAGE_PIXELS];
    for (int i = 0; i < numFullImageRecords; i++)
    {
        FullImagePacket *fip = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        FullImageContinuedPacket *cip = (FullImageContinuedPacket*)(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE);
        ImageAuxData aux;
        getImageData(fip, cip, &aux, pixels);
        printf("ISP %d:", i+1);
        printf(" EFI %d, sensor %d, meas. timestamp: %d", aux.EfiInstrumentId, aux.SensorNumber, fip->MeasurementTimestamp);
        printf(", CCD: %5.1lf C, V_FP: %5.2lf V", aux.CcdTemperature, aux.FaceplateVoltageMonitor);
        printf(", V_ID: %6.1lf V", aux.BiasGridVoltageMonitor);
        printf(", V_MCP: %7.1lf V", aux.McpVoltageMonitor);
        printf(", V_P: %7.1lf V", aux.PhosphorVoltageMonitor);
        printf("\n");
    }
    // Read all bytes

    // Confirm bytes read is what was expected


    // Loop through the images and print the image data
    // print voltages and other settings
    // Make png files with text overlays
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

int getLongValue(xmlDocPtr doc, const char * xpath, long *value)
{
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xpathCtx = xmlXPathNewContext(doc);
    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtx);
    if(xpathObj == NULL) 
    {
        fprintf(stderr,"Something's wrong, as I did not find any full image packets\n");
        xmlXPathFreeContext(xpathCtx); 
        return -1;
    }
    xmlNodeSetPtr nodeset = xpathObj->nodesetval;
    if (nodeset->nodeNr == 1)
    {
        *value = atol((char *)xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1));
    }
    return 0;

}


void print_xpath_nodes(xmlNodeSetPtr nodes) {
    xmlNodePtr cur;
    int size;
    int i;
    
    size = (nodes) ? nodes->nodeNr : 0;
    
    printf("Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i) {
	assert(nodes->nodeTab[i]);
	
	if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
	    xmlNsPtr ns;
	    
	    ns = (xmlNsPtr)nodes->nodeTab[i];
	    cur = (xmlNodePtr)ns->next;
	    if(cur->ns) { 
	        printf("= namespace \"%s\"=\"%s\" for node %s:%s\n", 
		    ns->prefix, ns->href, cur->ns->href, cur->name);
	    } else {
	        printf("= namespace \"%s\"=\"%s\" for node %s\n", 
		    ns->prefix, ns->href, cur->name);
	    }
	} else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
	    cur = nodes->nodeTab[i];   	    
	    if(cur->ns) { 
    	        printf("= element node \"%s:%s\"\n", 
		    cur->ns->href, cur->name);
	    } else {
    	        printf("= element node \"%s\"\n", 
		    cur->name);
	    }
	} else {
	    cur = nodes->nodeTab[i];    
	    printf("= node \"%s\": type %d\n", cur->name, cur->type);
	}
    }
}
