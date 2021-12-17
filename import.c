#include "import.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int importImagery(const char *hdr, HdrInfo *fi, HdrInfo *ci, uint8_t **fullImagePacketsPtr, uint8_t **continuedPacketsPtr)
{
    int status = IMPORT_OK;
    // get DBL filename and check that we can open it.
    char dbl[FILENAME_MAX];
    snprintf(dbl, strlen(hdr)-3, "%s", hdr);
    sprintf(dbl, "%s.DBL", dbl);

    uint8_t *fullImagePackets = NULL;
    uint8_t *continuedPackets = NULL;

    if (access(dbl, R_OK))
    {
        status = IMPORT_DBL_FILE_ACCESS;
        goto cleanup;
    }

    FILE *dblFile = fopen(dbl, "r");
    if (dblFile == NULL)
    {
        status = IMPORT_DBL_FILE_READ_PERMISION;
        goto cleanup;
    }
    long nImages = fi->numRecords;

    if (fi->numRecords < ci->numRecords)
    {
        // Increase buffer size to be able to display partial images
        nImages = ci->numRecords;
    }

    size_t fullImageBufferSize = (size_t) nImages * (size_t) fi->recordSize;
    fullImagePackets = (uint8_t*) malloc(fullImageBufferSize * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        status = IMPORT_DBL_BUFFER_ALLOCATION;
        goto cleanup;
    }
    size_t continuedBufferSize = (size_t) nImages * (size_t) ci->recordSize;
    continuedPackets = (uint8_t*) malloc(continuedBufferSize * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        status = IMPORT_DBL_BUFFER_ALLOCATION;
        goto cleanup;
    }

    // Bytes to read
    size_t fullImageTotalBytes = (size_t) fi->numRecords * (size_t) fi->recordSize;
    size_t continuedTotalBytes = (size_t) ci->numRecords * (size_t) ci->recordSize;

    size_t bytesRead = 0;
    // Set file offset to read full image packets
    if (fseek(dblFile, fi->offset, SEEK_SET))
    {
        status = IMPORT_DBL_FILE_SEEK;
        goto cleanup;
    }
    uint8_t * bufferStart = (uint8_t*)fullImagePackets;
    if (fi->numRecords < ci->numRecords)
    {
        bufferStart += (ci->numRecords - fi->numRecords) * fi->recordSize;
    }    
    if ((bytesRead = fread(bufferStart, sizeof(uint8_t), fullImageTotalBytes, dblFile)) != fullImageTotalBytes)
    {
        status = IMPORT_DBL_PACKET_READ;
        goto cleanup;
    }

    // Set file offset to read full image continued packets
    if (fseek(dblFile, ci->offset, SEEK_SET))
    {
        status = IMPORT_DBL_FILE_SEEK;
        goto cleanup;
    }
    bufferStart = (uint8_t*)continuedPackets;
    if (ci->numRecords < fi->numRecords)
    {
        bufferStart += (fi->numRecords - ci->numRecords) * ci->recordSize;
    }
    if ((bytesRead = fread((uint8_t*)bufferStart, sizeof(uint8_t), continuedTotalBytes, dblFile)) != continuedTotalBytes)
    {
        status = IMPORT_DBL_PACKET_READ;
        goto cleanup;
    }



cleanup:

    *fullImagePacketsPtr = fullImagePackets;
    *continuedPacketsPtr = continuedPackets;
    if (dblFile != NULL) fclose(dblFile);
    return status; 
}