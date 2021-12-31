#include "import.h"
#include "isp.h"

#include "xml.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fts.h>

int importImagery(const char *source, ImagePackets *imagePackets)
{
    int status = IMPORT_OK;
    int len = strlen(source);
    imagePackets->numberOfImages = 0;
    imagePackets->numberOfFullImagePackets = 0;
    imagePackets->numberOfContinuedPackets = 0;
    imagePackets->fullImagePackets = NULL;
    imagePackets->continuedPackets = NULL;
 
    if (strcmp(source + len - 4, ".HDR") == 0)
    {
        status = importImageryFromHdr(source, imagePackets);
        if (status != IMPORT_OK)
            return status;
    }
    else if (len == 9)
    {
        char *path[2] = {NULL, NULL};
        path[0] = ".";
        FTS * fts = fts_open(path, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
        if (fts == NULL)
            return IMPORT_SOURCE;
        FTSENT * f = fts_read(fts);
        int nameLength;
        char satellite = source[0];
        const char *date = source+1;
        while (f != NULL)
        {
            nameLength = strlen(f->fts_name);
            if ((strncmp(f->fts_name + nameLength - 40, date, 8) == 0 || strncmp(f->fts_name + nameLength - 24, date, 8) == 0) && f->fts_name[11] == satellite && strcmp(f->fts_name + nameLength - 4, ".HDR") == 0 && strncmp(f->fts_name, "SW_OPER_EFI", 11) == 0 && (strncmp(f->fts_name + nameLength - 47, "NOM_0__", 7) == 0 || strncmp(f->fts_name +nameLength - 47, "TIC_0__", 7) == 0) && nameLength == 59)
            {
                // Very likely got a SW_OPER_EFI?{NOM,TIC}_0__*.HDR file for the correct satellite and date
                status = importImageryFromHdr(f->fts_path, imagePackets);
                if (status != IMPORT_OK)
                {
                    fts_close(fts);
                    return status;
                }
            }
            f = fts_read(fts);
        }
        fts_close(fts);
    }
    else
        return IMPORT_SOURCE;
    
    // Align packets if there aren't the same number of full image and full image continued packets
    imagePackets->numberOfImages = imagePackets->numberOfFullImagePackets;
    if (imagePackets->numberOfContinuedPackets > imagePackets->numberOfFullImagePackets)
    {
        imagePackets->numberOfImages = imagePackets->numberOfContinuedPackets;
    }

    if (imagePackets->numberOfImages > 0)
    {
        long nGaps = numberOfPacketGaps(imagePackets->fullImagePackets, imagePackets->continuedPackets, imagePackets->numberOfImages);

        if (nGaps > 0)
        {
            imagePackets->numberOfImages += nGaps;
            imagePackets->fullImagePackets = realloc(imagePackets->fullImagePackets, imagePackets->numberOfImages * FULL_IMAGE_PACKET_SIZE);
            imagePackets->continuedPackets = realloc(imagePackets->continuedPackets, imagePackets->numberOfImages * FULL_IMAGE_CONT_PACKET_SIZE);

            alignPackets(imagePackets->fullImagePackets, imagePackets->continuedPackets, imagePackets->numberOfImages, nGaps);
        }
    }

    return status;
}


int importImageryFromHdr(const char *hdr, ImagePackets *imagePackets)
{
    int status = IMPORT_OK;

    if (strlen(hdr) >= FILENAME_MAX)
    {
        return IMPORT_HDR_FILENAME_TOO_LONG;
    }

    PacketFileContents pfc;
    status = parseHdr(hdr, &pfc);
    if (status)
    {
        return status;
    }

    // get DBL filename and check that we can open it.
    char dbl[FILENAME_MAX];
    snprintf(dbl, strlen(hdr)-3, "%s", hdr);
    sprintf(dbl + strlen(dbl), ".DBL");
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

    status = loadPackets(dblFile, &imagePackets->fullImagePackets, &imagePackets->numberOfFullImagePackets, &pfc.fullImage);
    if (status)
        goto cleanup;
    
    status = loadPackets(dblFile, &imagePackets->continuedPackets, &imagePackets->numberOfContinuedPackets, &pfc.fullImageContinued);
    if (status)
        goto cleanup;

cleanup:
    if (dblFile != NULL) fclose(dblFile);
    return status; 
}


void alignPackets(uint8_t* fullImagePackets, uint8_t *continuedPackets, long nImages, long nGaps)
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
        setDateTime(&(aux.dateTime), fip->DataFieldHeader);
        fdate = aux.dateTime.secondsSince1970;
        setDateTime(&(aux.dateTime), cip->DataFieldHeader);
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
        setDateTime(&(aux.dateTime), fip->DataFieldHeader);
        fdate = aux.dateTime.secondsSince1970;
        setDateTime(&(aux.dateTime), cip->DataFieldHeader);
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

int importScience(const char *source, SciencePackets *sciencePackets)
{
    int status = IMPORT_OK;

    // Init structure
    sciencePackets->lpTiiSciencePackets = NULL;
    sciencePackets->numberOfLpTiiSciencePackets = 0;
    sciencePackets->lpSweepPackets = NULL;
    sciencePackets->numberOfLpSweepPackets = 0;
    sciencePackets->configPackets = NULL;
    sciencePackets->numberOfConfigPackets = 0;
    sciencePackets->offsetPackets = NULL;
    sciencePackets->numberOfOffsetPackets = 0;

    int len = strlen(source);
    if (strcmp(source + len - 4, ".HDR") == 0)
    {
        status = importScienceFromHdr(source, sciencePackets);
        if (status != IMPORT_OK)
            return status;
    }
    else if (len == 9)
    {
        char *path[2] = {NULL, NULL};
        path[0] = ".";
        FTS * fts = fts_open(path, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
        if (fts == NULL)
            return IMPORT_SOURCE;
        FTSENT * f = fts_read(fts);
        int nameLength;
        char satellite = source[0];
        const char *date = source+1;
        while (f != NULL)
        {
            nameLength = strlen(f->fts_name);
            if ((strncmp(f->fts_name + nameLength - 40, date, 8) == 0 || strncmp(f->fts_name + nameLength - 24, date, 8) == 0) && f->fts_name[11] == satellite && strcmp(f->fts_name + nameLength - 4, ".HDR") == 0 && strncmp(f->fts_name, "SW_OPER_EFI", 11) == 0 && (strncmp(f->fts_name + nameLength - 47, "NOM_0__", 7) == 0 || strncmp(f->fts_name +nameLength - 47, "TIC_0__", 7) == 0) && nameLength == 59)
            {
                // Very likely got a SW_OPER_EFI?{NOM,TIC}_0__*.HDR file for the correct satellite and date
                status = importScienceFromHdr(f->fts_path, sciencePackets);
                if (status != IMPORT_OK)
                {
                    fts_close(fts);
                    return status;
                }
            }
            f = fts_read(fts);
        }
        fts_close(fts);
    }
    else
        return IMPORT_SOURCE;
    
    // TODO remove duplicate packets

    return status;
}

int importScienceFromHdr(const char *hdr, SciencePackets *sciencePackets)
{
    int status = IMPORT_OK;
    size_t bufSize = 0;
    size_t bytesRead = 0;

    if (strlen(hdr) >= FILENAME_MAX)
    {
        return IMPORT_HDR_FILENAME_TOO_LONG;
    }

    PacketFileContents pfc;
    status = parseHdr(hdr, &pfc);
    if (status)
    {
        return status;
    }

    // get DBL filename and check that we can open it.
    char dbl[FILENAME_MAX];
    snprintf(dbl, strlen(hdr)-3, "%s", hdr);
    sprintf(dbl + strlen(dbl), ".DBL");

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

    status = loadPackets(dblFile, &sciencePackets->lpTiiSciencePackets, &sciencePackets->numberOfLpTiiSciencePackets, &pfc.lpTiiScience);
    if (status)
        goto cleanup;

    status = loadPackets(dblFile, &sciencePackets->lpSweepPackets, &sciencePackets->numberOfLpSweepPackets, &pfc.lpSweep);
    if (status)
        goto cleanup;

    status = loadPackets(dblFile, &sciencePackets->offsetPackets, &sciencePackets->numberOfOffsetPackets, &pfc.lpOffset);
    if (status)
        goto cleanup;

    status = loadPackets(dblFile, &sciencePackets->configPackets, &sciencePackets->numberOfConfigPackets, &pfc.config);
    if (status)
        goto cleanup;


cleanup:
    if (dblFile != NULL) fclose(dblFile);
    return status; 
}


int arrayResize(uint8_t **array, size_t numRecords, size_t numNewRecords, size_t recordSize)
{
    size_t arrayBytes = numRecords * recordSize;
    size_t addedBytes = numNewRecords * recordSize;
    size_t newArrayBytes = arrayBytes + addedBytes;
    *array = (uint8_t*) realloc(*array, newArrayBytes);
    if (*array == NULL)
    {
        return IMPORT_DBL_BUFFER_ALLOCATION;
    }
    return IMPORT_OK;
}

int readBytes(uint8_t* array, FILE * dblFile, HdrInfo *info, size_t *bytesRead)
{
    size_t bytesToRead = info->numRecords * info->recordSize;
    size_t read = 0;
    if (fseek(dblFile, info->offset, SEEK_SET))
        return IMPORT_DBL_FILE_SEEK;
    if ((read = fread(array, sizeof(uint8_t), bytesToRead, dblFile)) != bytesToRead)
    {
        if (bytesRead != NULL) *bytesRead = 0;
        return IMPORT_DBL_PACKET_READ;
    }
    if (bytesRead != NULL) *bytesRead = 0;
    return IMPORT_OK;
}

int loadPackets(FILE *dblFile, uint8_t ** array, size_t *arrayNumRecords, HdrInfo *info)
{
    int status;

    status = arrayResize(array, *arrayNumRecords, info->numRecords, info->recordSize);
    if (*array == NULL)
    {
        return IMPORT_DBL_BUFFER_ALLOCATION;
    }
    // Append data to the array
    if (readBytes(*array + *arrayNumRecords * info->recordSize, dblFile, info, NULL))
    {
        return IMPORT_DBL_PACKET_READ;
    }
    *arrayNumRecords += info->numRecords;

    return IMPORT_OK;
}