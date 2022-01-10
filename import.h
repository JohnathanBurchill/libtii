#ifndef _IMPORT_H
#define _IMPORT_H

#include "isp.h"
#include "xml.h"

#include <stdint.h>

enum IMPORT_ERROR 
{
    IMPORT_OK = 0,
    IMPORT_DBL_FILE_ACCESS = -1,
    IMPORT_DBL_FILE_READ_PERMISION = -2,
    IMPORT_DBL_BUFFER_ALLOCATION = -3,
    IMPORT_DBL_FILE_SEEK = -4,
    IMPORT_DBL_PACKET_READ = -5,
    IMPORT_HDR_FILENAME_TOO_LONG = -6,
    IMPORT_SOURCE = -7,
    IMPORT_DATE_FORMAT = -8,
    IMPORT_DATE = -9
};

int importImagery(const char *source, ImagePackets *imagePackets);
int importImageryFromHdr(const char *hdr, ImagePackets *imagePackets);

void alignPackets(uint8_t* fullImagePackets, uint8_t *continuedPackets, long nImages, long nGaps);

int numberOfPacketGaps(uint8_t* fullImagePackets, uint8_t *continuedPackets, long nImages);

int importScience(const char *source, SciencePackets *SciencePackets);
int importScienceFromHdr(const char *hdr, SciencePackets *sciencePackets);

int comparePacketTimes(const void *p1, const void *p2);
int sortImagePackets(ImagePackets *imagePackets);
int sortSciencePackets(SciencePackets *sciencePackets);

int arrayResize(uint8_t **array, size_t numRecords, size_t numNewRecords, size_t recordSize);

int readBytes(uint8_t* array, FILE * dblFile, HdrInfo *info, size_t *bytesRead);

int loadPackets(FILE *dblFile, uint8_t ** array, size_t *arrayNumRecords, HdrInfo *info);

#endif // _IMPORT_H