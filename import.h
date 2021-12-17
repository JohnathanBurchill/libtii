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
    IMPORT_DBL_PACKET_READ = -5

};

int importImagery(const char *hdr, ImagePackets *imagePackets);

#endif // _IMPORT_H