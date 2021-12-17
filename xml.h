#ifndef _XML_H
#define _XML_H

#include <stdbool.h>
#include <libxml/parser.h>

typedef struct hdrInfo
{
    long offset;
    long numRecords;
    long recordSize;

} HdrInfo;

enum HDR_PARSE_ERR
{
    HDR_PARSE_OK = 0,
    HDR_PARSE_ERR_FULL_IMAGE_OFFSET = -1,
    HDR_PARSE_ERR_FULL_IMAGE_NUM_RECORDS = -2,
    HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE = -3,
    HDR_PARSE_ERR_FULL_IMAGE_CONT_OFFSET = -4,
    HDR_PARSE_ERR_FULL_IMAGE_CONT_NUM_RECORDS = -5,
    HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE = -6,
    HDR_PARSE_ERR_RECORD_SIZE = -7
};

int parseHdr(xmlDocPtr doc, HdrInfo *fi, HdrInfo *ci);
int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _XML_H
