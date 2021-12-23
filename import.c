#include "import.h"
#include "isp.h"

#include "xml.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

int importImagery(const char *hdr, ImagePackets *imagePackets)
{
    int status = IMPORT_OK;

    PacketFileContents pfc;
    status = parseHdr(hdr, &pfc);
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
    long nImages = pfc.fullImage.numRecords;
    long extraImages = pfc.fullImage.numRecords - pfc.fullImageContinued.numRecords;

    if (pfc.fullImage.numRecords < pfc.fullImageContinued.numRecords)
    {
        // Increase buffer size to be able to display partial images
        nImages = pfc.fullImageContinued.numRecords;
        extraImages = pfc.fullImageContinued.numRecords - pfc.fullImage.numRecords;
    }

    size_t fullImageBufferSize = (size_t) nImages * (size_t) pfc.fullImage.recordSize;
    fullImagePackets = (uint8_t*) malloc(fullImageBufferSize * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        status = IMPORT_DBL_BUFFER_ALLOCATION;
        goto cleanup;
    }
    size_t continuedBufferSize = (size_t) nImages * (size_t) pfc.fullImageContinued.recordSize;
    continuedPackets = (uint8_t*) malloc(continuedBufferSize * sizeof(uint8_t));
    if (fullImagePackets == NULL)
    {
        status = IMPORT_DBL_BUFFER_ALLOCATION;
        goto cleanup;
    }

    // Bytes to read
    size_t fullImageTotalBytes = (size_t) pfc.fullImage.numRecords * (size_t) pfc.fullImage.recordSize;
    size_t continuedTotalBytes = (size_t) pfc.fullImageContinued.numRecords * (size_t) pfc.fullImageContinued.recordSize;

    size_t bytesRead = 0;
    // Set file offset to read full image packets
    if (fseek(dblFile, pfc.fullImage.offset, SEEK_SET))
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
    if (fseek(dblFile, pfc.fullImageContinued.offset, SEEK_SET))
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
    long nGaps = numberOfPacketGaps(fullImagePackets, continuedPackets, nImages);
    // Already accounted for some extra images above; this account for hidden gaps (matched number of gaps in Full and continued packets)
    if (nGaps - extraImages > 0)
    {
        nImages += (nGaps - extraImages);
        fullImagePackets = realloc(fullImagePackets, nImages * FULL_IMAGE_PACKET_SIZE);
        continuedPackets = realloc(continuedPackets, nImages * FULL_IMAGE_CONT_PACKET_SIZE);
    }
    if (nGaps > 0)
    {
        alignPackets(fullImagePackets, continuedPackets, nImages, nGaps, &(pfc.fullImage), &(pfc.fullImageContinued));
    }



cleanup:
    imagePackets->numberOfImages = nImages;
    imagePackets->fullImagePackets = fullImagePackets;
    imagePackets->continuedPackets = continuedPackets;
    if (dblFile != NULL) fclose(dblFile);
    return status; 
}


void alignPackets(uint8_t* fullImagePackets, uint8_t *continuedPackets, long nImages, long nGaps, HdrInfo *fi, HdrInfo *ci)
{

    FullImagePacket *fip;
    FullImageContinuedPacket *cip;

    uint8_t *fipbuf = malloc(nImages * FULL_IMAGE_PACKET_SIZE);
    uint8_t *cipbuf = malloc(nImages * FULL_IMAGE_CONT_PACKET_SIZE);

    double fdate;
    double cdate;
    uint64_t fdateInt;
    uint64_t cdateInt;

    long correctedGaps = 0;
    long nF =0;
    long faultOffset = 0;
    IspDateTime *dt;
    ImageAuxData aux;

    for (long i = 0; i < nImages && correctedGaps < nGaps; i++)
    {
        fip = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        cip = (FullImageContinuedPacket*)(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE);
        setAuxDateTime(&aux, fip->DataFieldHeader);
        fdate = aux.dateTime.secondsSince1970;
        setAuxDateTime(&aux, cip->DataFieldHeader);
        cdate = aux.dateTime.secondsSince1970;
        fdateInt = *((uint64_t*)(fip->DataFieldHeader+4));
        cdateInt = *((uint64_t*)(cip->DataFieldHeader+4));
        if (fdate < cdate && fdateInt != 0)
        {
            memcpy(cipbuf, continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE, (nImages - i - 1)*FULL_IMAGE_CONT_PACKET_SIZE);
            memcpy(continuedPackets + (i+1)*FULL_IMAGE_CONT_PACKET_SIZE, cipbuf, (nImages - i - 1)*FULL_IMAGE_CONT_PACKET_SIZE);
            memset(continuedPackets + i*FULL_IMAGE_CONT_PACKET_SIZE, 0, FULL_IMAGE_CONT_PACKET_SIZE);
            correctedGaps++;
        }
        else if (fdate > cdate && cdateInt != 0)
        {
            memcpy(fipbuf, fullImagePackets + i*FULL_IMAGE_PACKET_SIZE, (nImages - i - 1)*FULL_IMAGE_PACKET_SIZE);
            memcpy(fullImagePackets + (i+1)*FULL_IMAGE_PACKET_SIZE, fipbuf, (nImages - i - 1)*FULL_IMAGE_PACKET_SIZE);
            memset(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE, 0, FULL_IMAGE_PACKET_SIZE);
            correctedGaps++;
        }
    }

    free(fipbuf);
    free(cipbuf);

}

int numberOfPacketGaps(uint8_t* fullImagePackets, uint8_t *continuedPackets, long nImages)
{
    long nGaps =0;
    long gapOffset = 0;
    IspDateTime *dt;
    ImageAuxData aux;

    FullImagePacket *fip;
    FullImageContinuedPacket *cip;

    double fdate;
    double cdate;
    uint64_t fdateInt;
    uint64_t cdateInt;

    for (long i = 0; i < nImages; i++)
    {
        fip = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        cip = (FullImageContinuedPacket*)(continuedPackets + (i+gapOffset)*FULL_IMAGE_CONT_PACKET_SIZE);
        setAuxDateTime(&aux, fip->DataFieldHeader);
        fdate = aux.dateTime.secondsSince1970;
        setAuxDateTime(&aux, cip->DataFieldHeader);
        cdate = aux.dateTime.secondsSince1970;
        fdateInt = *((uint64_t*)(fip->DataFieldHeader+4));
        cdateInt = *((uint64_t*)(cip->DataFieldHeader+4));
        if (fdate < cdate && floor(fdate) != 0)
        {
            nGaps++;
            gapOffset--;
        }
        else if (fdate > cdate && floor(cdate) != 0)
        {
            nGaps++;
            gapOffset++;
        }
    }
    return nGaps;
}
