/*

    TIIM processing library: lib/tii/xml.c

    Copyright (C) 2022  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "xml.h"

#include "isp.h"

#include <string.h>
#include <libxml/xpath.h>

int parseHdr(const char *hdr, PacketFileContents *packetInfo)
{
    // Set packetInfo to a known state
    packetInfo->config.offset = 0;
    packetInfo->config.numRecords = 0;
    packetInfo->config.recordSize = 0;
    packetInfo->lpTiiScience.offset = 0;
    packetInfo->lpTiiScience.numRecords = 0;
    packetInfo->lpTiiScience.recordSize = 0;
    packetInfo->lpSweep.offset = 0;
    packetInfo->lpSweep.numRecords = 0;
    packetInfo->lpSweep.recordSize = 0;
    packetInfo->lpOffset.offset = 0;
    packetInfo->lpOffset.numRecords = 0;
    packetInfo->lpOffset.recordSize = 0;


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

    char query[256];
    memset(query, 0, 256);
    int len = strlen(hdr);
    char mode[20];
    char bug;
    int modeValue = EFI_MODE_INVALID;
    memset(mode, 0, 20);
    if (strncmp(hdr+len-47, "TIC", 3) == 0)
    {
        sprintf(mode, "%s", "TII-Calibration");
        bug = '-';
        modeValue = EFI_MODE_TIICAL;
    }
    else
    {
        sprintf(mode, "%s", "Normal");
        bug = '\'';
        modeValue = EFI_MODE_NORMAL;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - %s Mode\"]/Data_Set_Offset", mode);
    if (getLongValue(doc, query, &(packetInfo->fullImage.offset)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_OFFSET;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - %s Mode\"]/Num_of_Records", mode);
    if (getLongValue(doc, query, &(packetInfo->fullImage.numRecords)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_NUM_RECORDS;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI TII Full Image Packet - %s Mode\"]/Record_Size", mode);
    if (getLongValue(doc, query, &(packetInfo->fullImage.recordSize)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont%cd Packet - %s Mode\"]/Data_Set_Offset", bug, mode);
    if (getLongValue(doc, query, &(packetInfo->fullImageContinued.offset)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_OFFSET;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont%cd Packet - %s Mode\"]/Num_of_Records", bug, mode);
    if (getLongValue(doc, query, &(packetInfo->fullImageContinued.numRecords)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_NUM_RECORDS;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI TII Full Image Cont%cd Packet - %s Mode\"]/Record_Size", bug, mode);
    if (getLongValue(doc, query, &(packetInfo->fullImageContinued.recordSize)))
    {
        status = HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE;
        goto cleanup;
    }

    // EFI config packet
    sprintf(query, "//DSD[Data_Set_Name=\"EFI Configuration Packet - %s Mode\"]/Data_Set_Offset", mode);
    if (getLongValue(doc, query, &(packetInfo->config.offset)))
    {
        status = HDR_PARSE_ERR_EFI_CONFIG_OFFSET;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI Configuration Packet - %s Mode\"]/Num_of_Records", mode);
    if (getLongValue(doc, query, &(packetInfo->config.numRecords)))
    {
        status = HDR_PARSE_ERR_EFI_CONFIG_NUM_RECORDS;
        goto cleanup;
    }
    sprintf(query, "//DSD[Data_Set_Name=\"EFI Configuration Packet - %s Mode\"]/Record_Size", mode);
    if (getLongValue(doc, query, &(packetInfo->config.recordSize)))
    {
        status = HDR_PARSE_ERR_EFI_CONFIG_RECORD_SIZE;
        goto cleanup;
    }

    // LP&TII Science packet
    if (modeValue == EFI_MODE_NORMAL)
    {
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP and TII Science Data - %s Mode\"]/Data_Set_Offset", mode);
        if (getLongValue(doc, query, &(packetInfo->lpTiiScience.offset)))
        {
            status = HDR_PARSE_ERR_LP_TII_SCIENCE_OFFSET;
            goto cleanup;
        }
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP and TII Science Data - %s Mode\"]/Num_of_Records", mode);
        if (getLongValue(doc, query, &(packetInfo->lpTiiScience.numRecords)))
        {
            status = HDR_PARSE_ERR_LP_TII_SCIENCE_NUM_RECORDS;
            goto cleanup;
        }
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP and TII Science Data - %s Mode\"]/Record_Size", mode);
        if (getLongValue(doc, query, &(packetInfo->lpTiiScience.recordSize)))
        {
            status = HDR_PARSE_ERR_LP_TII_SCIENCE_RECORD_SIZE;
            goto cleanup;
        }

        // LP Sweep
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP Sweep Packet - %s Mode\"]/Data_Set_Offset", mode);
        if (getLongValue(doc, query, &(packetInfo->lpSweep.offset)))
        {
            status = HDR_PARSE_ERR_LP_SWEEP_OFFSET;
            goto cleanup;
        }
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP Sweep Packet - %s Mode\"]/Num_of_Records", mode);
        if (getLongValue(doc, query, &(packetInfo->lpSweep.numRecords)))
        {
            status = HDR_PARSE_ERR_LP_SWEEP_NUM_RECORDS;
            goto cleanup;
        }
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP Sweep Packet - %s Mode\"]/Record_Size", mode);
        if (getLongValue(doc, query, &(packetInfo->lpSweep.recordSize)))
        {
            status = HDR_PARSE_ERR_LP_SWEEP_RECORD_SIZE;
            goto cleanup;
        }

        // LP Offset
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP Offset Packet - %s Mode\"]/Data_Set_Offset", mode);
        if (getLongValue(doc, query, &(packetInfo->lpOffset.offset)))
        {
            status = HDR_PARSE_ERR_LP_OFFSET_OFFSET;
            goto cleanup;
        }
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP Offset Packet - %s Mode\"]/Num_of_Records", mode);
        if (getLongValue(doc, query, &(packetInfo->lpOffset.numRecords)))
        {
            status = HDR_PARSE_ERR_LP_OFFSET_NUM_RECORDS;
            goto cleanup;
        }
        sprintf(query, "//DSD[Data_Set_Name=\"EFI LP Offset Packet - %s Mode\"]/Record_Size", mode);
        if (getLongValue(doc, query, &(packetInfo->lpOffset.recordSize)))
        {
            status = HDR_PARSE_ERR_LP_OFFSET_RECORD_SIZE;
            goto cleanup;
        }

    }
    else
    {
        packetInfo->config.offset = 0;
        packetInfo->config.numRecords = 0;
        packetInfo->config.recordSize = 0;
        packetInfo->lpTiiScience.offset = 0;
        packetInfo->lpTiiScience.numRecords = 0;
        packetInfo->lpTiiScience.recordSize = 0;
        packetInfo->lpSweep.offset = 0;
        packetInfo->lpSweep.numRecords = 0;
        packetInfo->lpSweep.recordSize = 0;
        packetInfo->lpOffset.offset = 0;
        packetInfo->lpOffset.numRecords = 0;
        packetInfo->lpOffset.recordSize = 0;
    }


    if (packetInfo->fullImage.numRecords > 0 && packetInfo->fullImage.recordSize != FULL_IMAGE_PACKET_SIZE)
    {
	fprintf(stderr, "parseHdr: incorrect FullImage RecordSize in %s\n", hdr);
        status = HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE;
        goto cleanup;
    }
    if (packetInfo->fullImageContinued.numRecords > 0 && packetInfo->fullImageContinued.recordSize != FULL_IMAGE_CONT_PACKET_SIZE)
    {
	fprintf(stderr, "parseHdr: incorrect FullImageContinued RecordSize in %s\n", hdr);
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
    // init return to 0 in case of failure to read tag
    *value = 0;
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
    if (nodeset && nodeset->nodeNr == 1)
    {
        *value = atol((char *)xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1));
    }
    return 0;

}
