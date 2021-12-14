#include "tiim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

    if (numFullImageRecords != numContinuedImageRecords)
    {
        printf("Incomplete images found. Skipping this file.\n");
        goto cleanup;
    }

    // get DBL filename and check that we can open it.
    
    // Set file offset to read full image packets

    // Read all bytes

    // Set file offset to read full image continued packets

    // Read all bytes

    // Confirm bytes read is what was expected


    // Loop through the images and print the image data
    // print voltages and other settings
    // Make png files with text overlays
    // Get the ion admittance from LP&TII packets and convert to density
    // Get config packet info as needed.




cleanup:

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
