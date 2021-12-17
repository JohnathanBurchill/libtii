#include "xml.h"

#include "isp.h"

#include <libxml/xpath.h>

int parseHdr(const char *hdr, HdrInfo *fi, HdrInfo *ci)
{

    int status = HDR_PARSE_OK;

    xmlInitParser();
    LIBXML_TEST_VERSION

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    if ((doc = xmlReadFile(hdr, NULL, 0)) == NULL)
    {
        status = HDR_PARSE_ERR_FILE_READ;
        goto cleanup;                
    }


    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Data_Set_Offset", &(fi->offset)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_OFFSET;
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Num_of_Records", &(fi->numRecords)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_NUM_RECORDS;
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Record_Size", &(fi->recordSize)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE;
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Data_Set_Offset", &(ci->offset)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_OFFSET;
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Num_of_Records", &(ci->numRecords)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_NUM_RECORDS;
        goto cleanup;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Record_Size", &(ci->recordSize)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE;
        goto cleanup;
    }

    if (fi->recordSize != FULL_IMAGE_PACKET_SIZE)
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE;
        goto cleanup;
    }
    if (ci->recordSize != FULL_IMAGE_CONT_PACKET_SIZE)
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE;
        goto cleanup;
    }


cleanup:

    if (doc != NULL) xmlFreeDoc(doc);
    xmlCleanupParser();

    return status;

}

int getLongValue(xmlDocPtr doc, const char * xpath, long *value)
{
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xpathCtx = xmlXPathNewContext(doc);
    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtx);
    if(xpathObj == NULL) 
    {
        fprintf(stderr,"I did not find what you are looking for.\n");
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
