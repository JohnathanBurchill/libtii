#include "xml.h"

#include "isp.h"

#include <libxml/xpath.h>

int parseHdr(xmlDocPtr doc, HdrInfo *fi, HdrInfo *ci)
{
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Data_Set_Offset", &(fi->offset)))
    {
        return HDR_PARSE_ERR_FULL_IMAGE_OFFSET;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Num_of_Records", &(fi->numRecords)))
    {
        return HDR_PARSE_ERR_FULL_IMAGE_NUM_RECORDS;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - Normal Mode\"]/Record_Size", &(fi->recordSize)))
    {
        return HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Data_Set_Offset", &(ci->offset)))
    {
        return HDR_PARSE_ERR_FULL_IMAGE_CONT_OFFSET;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Num_of_Records", &(ci->numRecords)))
    {
        return HDR_PARSE_ERR_FULL_IMAGE_CONT_NUM_RECORDS;
    }
    if (getLongValue(doc, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont'd Packet - Normal Mode\"]/Record_Size", &(ci->recordSize)))
    {
        return HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE;
    }

    if (fi->recordSize != FULL_IMAGE_PACKET_SIZE)
    {
        return HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE;
    }
    if (ci->recordSize != FULL_IMAGE_CONT_PACKET_SIZE)
    {
        return HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE;
    }

    return HDR_PARSE_OK;

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
