#include "import.h"
#include "isp.h"

#include "xml.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int importImagery(const char *hdr, ImagePackets *imagePackets)
{
    int status = IMPORT_OK;

    HdrInfo fi, ci;
    status = parseHdr(hdr, &fi, &ci);
    if (status)
    {
        return status;
    }

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
    long nImages = fi.numRecords;

    if (fi.numRecords < ci.numRecords)
    {
        // Increase buffer size to be able to display partial images
        nImages = ci.numRecords;
    }

    size_t fullImageBufferSize = (size_t) nImages * (size_t) fi.recordSize;
    fullImagePackets = (uint8_t*) malloc(fullImageBufferSize * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        status = IMPORT_DBL_BUFFER_ALLOCATION;
        goto cleanup;
    }
    size_t continuedBufferSize = (size_t) nImages * (size_t) ci.recordSize;
    continuedPackets = (uint8_t*) malloc(continuedBufferSize * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        status = IMPORT_DBL_BUFFER_ALLOCATION;
        goto cleanup;
    }

    // Bytes to read
    size_t fullImageTotalBytes = (size_t) fi.numRecords * (size_t) fi.recordSize;
    size_t continuedTotalBytes = (size_t) ci.numRecords * (size_t) ci.recordSize;

    size_t bytesRead = 0;
    // Set file offset to read full image packets
    if (fseek(dblFile, fi.offset, SEEK_SET))
    {
        status = IMPORT_DBL_FILE_SEEK;
        goto cleanup;
    }
    uint8_t * bufferStart = (uint8_t*)fullImagePackets;
    if ((bytesRead = fread(bufferStart, sizeof(uint8_t), fullImageTotalBytes, dblFile)) != fullImageTotalBytes)
    {
        status = IMPORT_DBL_PACKET_READ;
        goto cleanup;
    }

    // Set file offset to read full image continued packets
    if (fseek(dblFile, ci.offset, SEEK_SET))
    {
        status = IMPORT_DBL_FILE_SEEK;
        goto cleanup;
    }
    bufferStart = (uint8_t*)continuedPackets;
    if ((bytesRead = fread((uint8_t*)bufferStart, sizeof(uint8_t), continuedTotalBytes, dblFile)) != continuedTotalBytes)
    {
        status = IMPORT_DBL_PACKET_READ;
        goto cleanup;
    }

    // Align packets if there aren't the same number of full image and full image continued packets
    if (ci.numRecords != fi.numRecords)
    {
        alignPackets(fullImagePackets, continuedPackets, nImages, &fi, &ci);
    }



cleanup:
    imagePackets->numberOfImages = nImages;
    imagePackets->fullImagePackets = fullImagePackets;
    imagePackets->continuedPackets = continuedPackets;
    if (dblFile != NULL) fclose(dblFile);
    return status; 
}


void alignPackets(uint8_t* fullImagePackets, uint8_t *continuedPackets, long nImages, HdrInfo *fi, HdrInfo *ci)
{
    FullImagePacket *fip;
    FullImageContinuedPacket *cip;

    uint8_t *fipbuf = malloc(nImages * FULL_IMAGE_PACKET_SIZE);
    uint8_t *cipbuf = malloc(nImages * FULL_IMAGE_CONT_PACKET_SIZE);

    uint64_t fdate;
    uint64_t cdate;

    long numFaults = fi->numRecords > ci->numRecords ? fi->numRecords - ci->numRecords : ci->numRecords - fi->numRecords;

    long correctedFaults = 0;

    for (long i = 0; i < nImages && correctedFaults < numFaults; i++)
    {
        fip = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        cip = (FullImageContinuedPacket*)(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE);
        fdate = *((uint64_t*)(fip->DataFieldHeader+4));
        cdate = *((uint64_t*)(cip->DataFieldHeader+4));
        if (fdate < cdate && fdate != 0)
        {
            memcpy(cipbuf, continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE, (nImages - i - 1)*FULL_IMAGE_CONT_PACKET_SIZE);
            memcpy(continuedPackets + (i+1)*FULL_IMAGE_CONT_PACKET_SIZE, cipbuf, (nImages - i - 1)*FULL_IMAGE_CONT_PACKET_SIZE);
            memset(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE, 0, FULL_IMAGE_CONT_PACKET_SIZE);
            correctedFaults++;
        }
        else if (fdate > cdate && cdate != 0)
        {
            memcpy(fipbuf, fullImagePackets + i*FULL_IMAGE_PACKET_SIZE, (nImages - i - 1)*FULL_IMAGE_PACKET_SIZE);
            memcpy(fullImagePackets + (i+1)*FULL_IMAGE_PACKET_SIZE, fipbuf, (nImages - i - 1)*FULL_IMAGE_PACKET_SIZE);
            memset(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE, 0, FULL_IMAGE_PACKET_SIZE);
            correctedFaults++;
        }
    }

    free(fipbuf);
    free(cipbuf);
}


