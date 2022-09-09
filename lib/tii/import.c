/*

    TIIM processing library: lib/tii/import.c

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

#include "import.h"
#include "isp.h"
#include "utility.h"

#include "xml.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fts.h>

int sortFiles(const FTSENT **first, const FTSENT **second)
{
    if (((*first)->fts_namelen != 59) || (*second)->fts_namelen != 59)
        return 0;
    else 
        return strncmp((*first)->fts_name + 19, (*second)->fts_name + 19, 15);
}

int importImagery(const char *source, ImagePackets *imagePackets)
{
    return importImageryWithFilenames(source, imagePackets, NULL, NULL);
}

int importImageryWithFilenames(const char *source, ImagePackets *imagePackets, char **efiFilenames, size_t *nFiles)
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
        FTS * fts = fts_open(path, FTS_PHYSICAL | FTS_NOCHDIR, &sortFiles);
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
                // Ignore errors: we read as many files as we can
                status = importImageryFromHdr(f->fts_path, imagePackets);
                if (status == IMPORT_OK && nFiles != NULL)
                {
                    *efiFilenames = (char *)realloc(*efiFilenames, FILENAME_MAX * (*nFiles + 1));
                    if (*efiFilenames != NULL)
                    {
                        sprintf(*efiFilenames + (*nFiles)*FILENAME_MAX, "%s", f->fts_path);
                        sprintf(*efiFilenames + (*nFiles)*FILENAME_MAX + strlen(f->fts_path)-3, "DBL");
                        (*nFiles)++;
                    }
                }
            }
            f = fts_read(fts);
        }
        fts_close(fts);
    }
    else
        return IMPORT_SOURCE;
    
    sortImagePackets(imagePackets);

    // Align packets if there aren't the same number of full image and full image continued packets
    imagePackets->numberOfImages = imagePackets->numberOfFullImagePackets;
    // if (imagePackets->numberOfContinuedPackets > imagePackets->numberOfFullImagePackets)
    // {
    //     imagePackets->numberOfImages = imagePackets->numberOfContinuedPackets;
    // }

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

    for (long i = 0; (i < nImages) && ((i + gapOffset) < nImages); i++)
    {
        fip = (FullImagePacket*)(fullImagePackets + i*FULL_IMAGE_PACKET_SIZE);
        setDateTime(&(aux.dateTime), fip->DataFieldHeader);
        fdate = aux.dateTime.secondsSince1970;
        fdateInt = *((uint64_t*)(fip->DataFieldHeader+4));

        cip = (FullImageContinuedPacket*)(continuedPackets + (i+gapOffset)*FULL_IMAGE_CONT_PACKET_SIZE);
        setDateTime(&(aux.dateTime), cip->DataFieldHeader);
        cdate = aux.dateTime.secondsSince1970;
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
        FTS * fts = fts_open(path, FTS_PHYSICAL | FTS_NOCHDIR, &sortFiles);
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
                // Ignore errors here: we try to return data from at least a subset of valid files
                status = importScienceFromHdr(f->fts_path, sciencePackets);
            }
            f = fts_read(fts);
        }
        fts_close(fts);
    }
    else
        return IMPORT_SOURCE;
    
    sortSciencePackets(sciencePackets);
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

    if (pfc.lpTiiScience.numRecords > 0)
    {
        status = loadPackets(dblFile, &sciencePackets->lpTiiSciencePackets, &sciencePackets->numberOfLpTiiSciencePackets, &pfc.lpTiiScience);
        if (status)
            goto cleanup;
    }

    if (pfc.lpSweep.numRecords > 0)
    {
        status = loadPackets(dblFile, &sciencePackets->lpSweepPackets, &sciencePackets->numberOfLpSweepPackets, &pfc.lpSweep);
        if (status)
            goto cleanup;
    }

    if (pfc.lpOffset.numRecords > 0)
    {
        status = loadPackets(dblFile, &sciencePackets->offsetPackets, &sciencePackets->numberOfOffsetPackets, &pfc.lpOffset);
        if (status)
            goto cleanup;
    }

    if (pfc.config.numRecords > 0)
    {
        status = loadPackets(dblFile, &sciencePackets->configPackets, &sciencePackets->numberOfConfigPackets, &pfc.config);
        if (status)
            goto cleanup;
    }

cleanup:
    if (dblFile != NULL) fclose(dblFile);
    return status; 
}

int comparePacketTimes(const void *p1, const void *p2)
{
    // offsets for bytes 5 through 12 of each packet's data field header
    uint8_t *cds1 = ((uint8_t*) p1) + 30;
    uint8_t *cds2 = ((uint8_t*) p2) + 30;

    double day1 = (double) (cds1[0]*256 + cds1[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms1 = (double) (cds1[2]*256*256*256 + cds1[3]*256*256 + cds1[4]*256 + cds1[5]);
    double us1 = (double) (cds1[6]*256 + cds1[7]);
    double t1 = day1 * 86400. + ms1 / 1.0e3 + us1 / 1.0e6;

    double day2 = (double) (cds2[0]*256 + cds2[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms2 = (double) (cds2[2]*256*256*256 + cds2[3]*256*256 + cds2[4]*256 + cds2[5]);
    double us2 = (double) (cds2[6]*256 + cds2[7]);
    double t2 = day2 * 86400. + ms2 / 1.0e3 + us2 / 1.0e6;

    if (t1 > t2)
        return 1;
    else if (t1 == t2)
        return 0;
    else
        return -1;

}

int compareFullImagePacketTimes(const void *p1, const void *p2)
{
    // offsets for bytes 5 through 12 of each packet's data field header
    uint8_t *cds1 = ((uint8_t*) p1) + 30;
    uint8_t *cds2 = ((uint8_t*) p2) + 30;

    double day1 = (double) (cds1[0]*256 + cds1[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms1 = (double) (cds1[2]*256*256*256 + cds1[3]*256*256 + cds1[4]*256 + cds1[5]);
    double us1 = (double) (cds1[6]*256 + cds1[7]);
    double t1 = day1 * 86400. + ms1 / 1.0e3 + us1 / 1.0e6;

    double day2 = (double) (cds2[0]*256 + cds2[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms2 = (double) (cds2[2]*256*256*256 + cds2[3]*256*256 + cds2[4]*256 + cds2[5]);
    double us2 = (double) (cds2[6]*256 + cds2[7]);
    double t2 = day2 * 86400. + ms2 / 1.0e3 + us2 / 1.0e6;

    // If the packet times are the same, make sure the H sensor comes first
    if (t1 > t2)
        return 1;
    else if (t1 == t2)
    {
        uint8_t sensor1 = ((*(((uint8_t*) p1) + 48)) >> 1) & 0x1;
        uint8_t sensor2 = ((*(((uint8_t*) p2) + 48)) >> 1) & 0x1;
        if (sensor1 > sensor2)
            return 1;
        else if (sensor1 < sensor2)
            return -1;
        else
            return 0;
    }
    else
        return -1;

}

int compareContinuedImagePacketTimes(const void *p1, const void *p2)
{
    // offsets for bytes 5 through 12 of each packet's data field header
    uint8_t *cds1 = ((uint8_t*) p1) + 30;
    uint8_t *cds2 = ((uint8_t*) p2) + 30;

    double day1 = (double) (cds1[0]*256 + cds1[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms1 = (double) (cds1[2]*256*256*256 + cds1[3]*256*256 + cds1[4]*256 + cds1[5]);
    double us1 = (double) (cds1[6]*256 + cds1[7]);
    double t1 = day1 * 86400. + ms1 / 1.0e3 + us1 / 1.0e6;

    double day2 = (double) (cds2[0]*256 + cds2[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms2 = (double) (cds2[2]*256*256*256 + cds2[3]*256*256 + cds2[4]*256 + cds2[5]);
    double us2 = (double) (cds2[6]*256 + cds2[7]);
    double t2 = day2 * 86400. + ms2 / 1.0e3 + us2 / 1.0e6;

    // If the packet times are the same, make sure the H sensor comes first
    if (t1 > t2)
        return 1;
    else if (t1 == t2)
    {
        uint8_t sensor1 = ((*(((uint8_t*) p1) + 39)) >> 7) & 0x1;
        uint8_t sensor2 = ((*(((uint8_t*) p2) + 39)) >> 7) & 0x1;
        if (sensor1 > sensor2)
            return 1;
        else if (sensor1 < sensor2)
            return -1;
        else
            return 0;
    }
    else
        return -1;

}

void sortImagePackets(ImagePackets *imagePackets)
{
    qsort(imagePackets->fullImagePackets, imagePackets->numberOfFullImagePackets, FULL_IMAGE_PACKET_SIZE, &compareFullImagePacketTimes);
    qsort(imagePackets->continuedPackets, imagePackets->numberOfContinuedPackets, FULL_IMAGE_CONT_PACKET_SIZE, &compareContinuedImagePacketTimes);
}

void sortSciencePackets(SciencePackets *sciencePackets)
{
    qsort(sciencePackets->lpTiiSciencePackets, sciencePackets->numberOfLpTiiSciencePackets, LP_TII_SCIENCE_PACKET_SIZE, &comparePacketTimes);
    qsort(sciencePackets->configPackets, sciencePackets->numberOfConfigPackets, CONFIG_PACKET_SIZE, &comparePacketTimes);
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
