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

typedef struct packetFileContents
{
    HdrInfo fullImage;
    HdrInfo fullImageContinued;
    HdrInfo efiConfig;
    HdrInfo lpTiiScience;
    HdrInfo lpSweep;
    HdrInfo lpOffset;
} PacketFileContents;

enum HDR_PARSE_ERR
{
    HDR_PARSE_OK = 0,
    HDR_PARSE_ERR_FULL_IMAGE_OFFSET = -1,
    HDR_PARSE_ERR_FULL_IMAGE_NUM_RECORDS = -2,
    HDR_PARSE_ERR_FULL_IMAGE_RECORD_SIZE = -3,
    HDR_PARSE_ERR_FULL_IMAGE_CONT_OFFSET = -4,
    HDR_PARSE_ERR_FULL_IMAGE_CONT_NUM_RECORDS = -5,
    HDR_PARSE_ERR_FULL_IMAGE_CONT_RECORD_SIZE = -6,
    HDR_PARSE_ERR_RECORD_SIZE = -7,
    HDR_PARSE_ERR_FILE_READ = -8,
    HDR_PARSE_ERR_EFI_CONFIG_OFFSET = -9,
    HDR_PARSE_ERR_EFI_CONFIG_NUM_RECORDS = -10,
    HDR_PARSE_ERR_EFI_CONFIG_RECORD_SIZE = -11,
    HDR_PARSE_ERR_LP_TII_SCIENCE_OFFSET = -12,
    HDR_PARSE_ERR_LP_TII_SCIENCE_NUM_RECORDS = -13,
    HDR_PARSE_ERR_LP_TII_SCIENCE_RECORD_SIZE = -14,
    HDR_PARSE_ERR_LP_SWEEP_OFFSET = -15,
    HDR_PARSE_ERR_LP_SWEEP_NUM_RECORDS = -16,
    HDR_PARSE_ERR_LP_SWEEP_RECORD_SIZE = -17,
    HDR_PARSE_ERR_LP_OFFSET_OFFSET = -18,
    HDR_PARSE_ERR_LP_OFFSET_NUM_RECORDS = -19,
    HDR_PARSE_ERR_LP_OFFSET_RECORD_SIZE = -20,
};

int parseHdr(const char *hdr, PacketFileContents *packetInfo);
int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _XML_H
