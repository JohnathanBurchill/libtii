#include "isp.h"

// #include "tiim.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void getImageData(FullImagePacket * fip, FullImageContinuedPacket * cip, ImageAuxData * aux, uint16_t *pixels)
{

    uint8_t * fullBytes = fip->AuxData;
    uint8_t * contBytes = cip->AuxData;
    char efiUnit[4] = {'X', 'C', 'B', 'A'};

    // Init pixels to zeros
    memset((void*)pixels, 0, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));

    uint8_t contSensor = (contBytes[0] >> 7) & 0x01;
    uint8_t contGainTableId = ((contBytes[0] & 0x03) << 2) | (contBytes[1] >> 6);
    uint8_t contEfiInstrumentId = ((contBytes[1] >> 2) & 0x0f);

    aux->SensorNumber = H_SENSOR;
    aux->sensor = 0;
    aux->GainTableId = 0;
    aux->EfiInstrumentId = 'X';
    uint8_t zeros[12] = {0};
    setDateTime(&(aux->dateTime), zeros);

    uint64_t fdate = *((uint64_t*)(fip->DataFieldHeader+4));
    uint64_t cdate = *((uint64_t*)(cip->DataFieldHeader+4));

    if (fip->StructureId == ISP_TYPE_FULL_IMAGE)
    {
        aux->SensorNumber = (fullBytes[9] >> 1) & 0x01;
        aux->sensor = aux->SensorNumber ? 'V' : 'H';
        aux->GainTableId = ((fullBytes[11] & 0x01) << 3) | (fullBytes[12] >> 5);
        aux->EfiInstrumentId = (fullBytes[12] >> 1) & 0x0f;
        aux->satellite = efiUnit[aux->EfiInstrumentId];
        setDateTime(&(aux->dateTime), fip->DataFieldHeader);

        aux->CcdDarkCurrent = (fullBytes[0] << 4) | (fullBytes[1] >> 4);
        aux->CcdTemperature = 259. * pow(aux->CcdDarkCurrent, 0.0383) - 273.15;
        if (aux->CcdTemperature < -35.0) aux->CcdTemperature = -35.0;
        if (aux->CcdTemperature > 50.0) aux->CcdTemperature = 50.0;
        aux->FaceplateVoltageMonitorRaw = ((fullBytes[1] << 10) & 0xff) | ((fullBytes[2] << 2) & 0xff) | (fullBytes[3]>>6);
        aux->FaceplateVoltageMonitor = -5.0 + (double)aux->FaceplateVoltageMonitorRaw / 255.0 * 5.0;
        aux->BiasGridVoltageMonitorRaw = ((fullBytes[3] & 0x3f) << 6) | ((fullBytes[4] >> 2) & 0x3f);
        aux->BiasGridVoltageMonitor = - (double) aux->BiasGridVoltageMonitorRaw / 4095.0 * 100.0 * 4.0 / 2.5;
        aux->ShutterDutyCycleRaw = ((fullBytes[4] & 0x3) << 14) | (fullBytes[5] << 6) | ((fullBytes[6] >> 2) & 0x3f);
        aux->ShutterDutyCycle = 1.0 - aux->ShutterDutyCycleRaw / (52031.0/0.999); // 52031 corresponds to 0.1% open
        aux->McpVoltageMonitorRaw = ((fullBytes[6] & 0x3) << 10) | (fullBytes[7] << 2) | ((fullBytes[8] >> 6) & 0x3);
        aux->McpVoltageMonitor = - (double) aux->McpVoltageMonitorRaw / 4095.0 * 2400.0 * 4.0 / 2.5;
        aux->PhosphorVoltageMonitorRaw = ((fullBytes[8] & 0x3f) << 6) | ((fullBytes[9] >> 2) & 0x3f);
        aux->PhosphorVoltageMonitor = aux->PhosphorVoltageMonitorRaw / 4095.0 * 8000.0 * 4.0 / 2.5;
        // Populate full image packet pixels
        // All this to extract packed 12 bit pixels
        // Two pixels in three bytes
        int b = 0;
        int p = 0;
        for (int i = 0; i < NUM_FULL_IMAGE_PACKET_PIXELS-1; i+=2)
        {
            pixels[p++] = (fip->PixelBytes[b] << 4) | ((fip->PixelBytes[b+1] >> 4) & 0x0f);
            pixels[p++] = ((fip->PixelBytes[b+1] & 0x0f) << 8) | fip->PixelBytes[b+2];
            b+=3;
        }
        pixels[p++] = (fip->PixelBytes[b] << 4) | ((fip->PixelBytes[b+1] >> 4) & 0x0f);
    }
    else
    {
        aux->CcdDarkCurrent = 0;
        aux->CcdTemperature = 0;
        aux->FaceplateVoltageMonitor = 0;
        aux->FaceplateVoltageMonitorRaw = 0;
        aux->BiasGridVoltageMonitor = 0;
        aux->BiasGridVoltageMonitorRaw = 0;
        aux->ShutterDutyCycle = 0;
        aux->ShutterDutyCycleRaw = 0;
        aux->McpVoltageMonitor = 0;
        aux->McpVoltageMonitorRaw = 0;
        aux->PhosphorVoltageMonitor = 0;
        aux->PhosphorVoltageMonitorRaw = 0;
        if (cip->StructureId == ISP_TYPE_FULL_IMAGE_CONTINUED) // Probably the case, unless both packets are bad
        {
            aux->SensorNumber = contSensor;
            aux->sensor = aux->SensorNumber ? 'V' : 'H';
            aux->GainTableId = contGainTableId;
            aux->EfiInstrumentId = contEfiInstrumentId;
            aux->satellite = efiUnit[aux->EfiInstrumentId];
            // Set time to Full Image Continued time
            fdate = *((uint64_t*)(cip->DataFieldHeader+4));
            setDateTime(&(aux->dateTime), cip->DataFieldHeader);
        }
    }
    // Get Full Image Continued Image data if the continued packet is good and the times and sensors match (will match if there was a missing Full Image Packet)
    if (cip->StructureId == ISP_TYPE_FULL_IMAGE_CONTINUED && fdate == cdate && aux->SensorNumber == contSensor)
    {
        // Got the right continued packet, finish the image
        int b = 0;
        int p = 1341; // Known offset; needed in the Full Image data were not present
        for (int i = 0; i < NUM_FULL_IMAGE_CONT_PACKET_PIXELS-1; i+=2)
        {
            pixels[p++] = (cip->PixelBytes[b] << 4) | ((cip->PixelBytes[b+1] >> 4) & 0x0f);
            pixels[p++] = ((cip->PixelBytes[b+1] & 0x0f) << 8) | cip->PixelBytes[b+2];
            b+=3;
        }
        pixels[p] = (cip->PixelBytes[b] << 4) | ((cip->PixelBytes[b+1] >> 4) & 0x0f);
        // times and sensors match; we have a consistent image if the Full Image Packet was present
        aux->consistentImage = fip->StructureId == ISP_TYPE_FULL_IMAGE;
    }
 
    return;

}


void getImagePair(ImagePackets *imagePackets, size_t packetIndex, ImagePair *imagePair)
{
    uint8_t *fullImagePackets = imagePackets->fullImagePackets;
    uint8_t *continuedPackets = imagePackets->continuedPackets;
    size_t numberOfImages = imagePackets->numberOfImages;

    FullImagePacket *fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;

    imagePair->gotImageH = false;
    imagePair->gotImageV = false;

    if (packetIndex < numberOfImages)
    {
        fip1 = (FullImagePacket*)(fullImagePackets + packetIndex*FULL_IMAGE_PACKET_SIZE);
        cip1 = (FullImageContinuedPacket*)(continuedPackets + packetIndex*FULL_IMAGE_CONT_PACKET_SIZE);
        getImageData(fip1, cip1, imagePair->auxH, imagePair->pixelsH);
        imagePair->gotImageH = true;
    }
    else
    {
        memset(imagePair->pixelsH, 0, NUM_FULL_IMAGE_PIXELS * 2);
        imagePair->auxH->EfiInstrumentId = UNIT_INVALID;
        return;
    }
    if ((packetIndex + 1) < numberOfImages)
    {
        fip2 = (FullImagePacket*)(fullImagePackets + (packetIndex+1)*FULL_IMAGE_PACKET_SIZE);
        cip2 = (FullImageContinuedPacket*)(continuedPackets + (packetIndex+1)*FULL_IMAGE_CONT_PACKET_SIZE);
        getImageData(fip2, cip2, imagePair->auxV, imagePair->pixelsV);
        imagePair->gotImageV = true;
    }
    else
    {
        memset(imagePair->pixelsV, 0, NUM_FULL_IMAGE_PIXELS * 2);
        imagePair->auxV->EfiInstrumentId = UNIT_INVALID;
   }
}


int alignImages(ImagePair *imagePair)
{
    int status = ISP_ALIGNED_IMAGE_PAIR;

    if (!(imagePair->gotImageH) && !(imagePair->gotImageV))
    {
        status = ISP_NO_IMAGE_PAIR;
    }
    else if (imagePair->auxH->SensorNumber == H_SENSOR && imagePair->auxV->SensorNumber == V_SENSOR)
    {
        status = ISP_ALIGNED_IMAGE_PAIR;
    }
    else if (imagePair->auxH->SensorNumber == V_SENSOR)
    {
        imagePair->gotImageH = false;
        imagePair->gotImageV = true;
        // We got a V image first. Draw an empty H image.
        // Swap H and V and zero out H pixels
        uint16_t *buf = imagePair->pixelsV;
        imagePair->pixelsV = imagePair->pixelsH;
        imagePair->pixelsH = buf;
        memset(imagePair->pixelsH, MISSING_PIXEL_VALUE, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));
        ImageAuxData *abuf = imagePair->auxV;
        imagePair->auxV = imagePair->auxH;
        imagePair->auxH = abuf;
        status = ISP_V_IMAGE;
    }
    else if (imagePair->auxH->SensorNumber == H_SENSOR && imagePair->auxV->SensorNumber == 0)
    {
        imagePair->gotImageH = true;
        imagePair->gotImageV = false;
        // Zero out V pixels
        memset(imagePair->pixelsV, MISSING_PIXEL_VALUE, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));
        status = ISP_H_IMAGE;
    }
    return status;
}

int getAlignedImagePair(ImagePackets *imagePackets, size_t packetIndex, ImagePair *imagePair, int *imagesRead)
{
    int status = 0;
    getImagePair(imagePackets, packetIndex, imagePair);
    status = alignImages(imagePair);

    if (status == ISP_ALIGNED_IMAGE_PAIR)
        *imagesRead = 2;
    else if (status == ISP_H_IMAGE || status == ISP_V_IMAGE)
        *imagesRead = 1;
    else
        *imagesRead = 0;

    if (imagePair->gotImageH)
        imagePair->secondsSince1970 = imagePair->auxH->dateTime.secondsSince1970;
    else
        imagePair->secondsSince1970 = imagePair->auxV->dateTime.secondsSince1970;

    return status;
}


int getFirstImagePair(ImagePackets *imagePackets, ImagePair *imagePair)
{
    int status;
    int packetIndex = 0;
    int imagesRead = 0;

    while((status = getAlignedImagePair(imagePackets, packetIndex++, imagePair, &imagesRead)) == ISP_NO_IMAGE_PAIR && packetIndex < imagePackets->numberOfImages);

    return status;

}

int getLastImagePair(ImagePackets *imagePackets, ImagePair *imagePair)
{
    int status;
    int packetIndex = imagePackets->numberOfImages - 1;
    int imagesRead = 0;

    while((status = getAlignedImagePair(imagePackets, packetIndex--, imagePair, &imagesRead)) == ISP_NO_IMAGE_PAIR && packetIndex >= 0);

    return status;

}

IspDateTime * getIspDateTime(ImagePair *imagePair)
{
    // Use H image time if available, otherwise V image time
    // They are the same, but handle case where H image is unvailable
    if (imagePair->gotImageH)
        return &(imagePair->auxH->dateTime);
    else if (imagePair->gotImageV)
        return &(imagePair->auxV->dateTime);
    else
        return NULL;

}

char getSatellite(ImagePair *imagePair)
{
    // Use H data if available, otherwise V data
    // They are the same, but handle case where H is unvailable
    if (imagePair->gotImageH)
        return imagePair->auxH->satellite;
    else if (imagePair->gotImageV)
        return imagePair->auxV->satellite;
    else
        return 'X';

}

void initializeImagePair(ImagePair *imagePair, ImageAuxData *auxH, uint16_t *pixelsH, ImageAuxData *auxV, uint16_t *pixelsV)
{
    imagePair->auxH = auxH;
    imagePair->auxV = auxV;
    imagePair->pixelsH = pixelsH;
    imagePair->pixelsV = pixelsV;
    imagePair->secondsSince1970 = 0;
}

void setDateTime(IspDateTime *dateTime, uint8_t *dataFieldHeader)
{
    // ISP time (epoch 2000-01-01 00:00:00 UT)
    uint8_t * cds = dataFieldHeader+4;
    double day = (double) (cds[0]*256 + cds[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms = (double) (cds[2]*256*256*256 + cds[3]*256*256 + cds[4]*256 + cds[5]);
    double us = (double) (cds[6]*256 + cds[7]);
    double secondsSince1970 = day * 86400. + ms / 1.0e3 + us / 1.0e6;
    time_t seconds = floor(secondsSince1970);
    struct tm *timeStruct = gmtime(&seconds);
    dateTime->secondsSince1970 = secondsSince1970;
    dateTime->year = timeStruct->tm_year + 1900;
    dateTime->month = timeStruct->tm_mon + 1;
    dateTime->day = timeStruct->tm_mday;
    dateTime->hour = timeStruct->tm_hour;
    dateTime->minute = timeStruct->tm_min;
    dateTime->second = timeStruct->tm_sec;
    dateTime->millisecond = floor((secondsSince1970 - (double)seconds)*1000);

}

void getLpTiiScienceData(LpTiiSciencePacket * pkt, LpTiiScience * science)
{

    uint8_t *bytes = pkt->LpTiiData;
    double di;
    uint8_t b[2];

    // Aux data
    science->BiasGridVoltageSettingRawH = pkt->AuxDataH[3];
    science->BiasGridVoltageSettingH = ((double)science->BiasGridVoltageSettingRawH) / 255. * -100.0;
    science->McpVoltageSettingRawH = pkt->AuxDataH[4];
    science->McpVoltageSettingH = ((double)science->McpVoltageSettingRawH) / 255. * (-2400.0);
    science->PhosphorVoltageSettingRawH = pkt->AuxDataH[5];
    science->PhosphorVoltageSettingH = ((double)science->PhosphorVoltageSettingRawH) / 255. * 8000.0;
    
    science->BiasGridVoltageSettingRawV = pkt->AuxDataV[3];
    science->BiasGridVoltageSettingV = ((double)science->BiasGridVoltageSettingRawV) / 255. * -100.0;
    science->McpVoltageSettingRawV = pkt->AuxDataV[4];
    science->McpVoltageSettingV = ((double)science->McpVoltageSettingRawV) / 255. * (-2400.0);
    science->PhosphorVoltageSettingRawV = pkt->AuxDataV[5];
    science->PhosphorVoltageSettingV = ((double)science->PhosphorVoltageSettingRawV) / 255. * 8000.0;

    // 2nd y moment
    science->Y2H[0] = (double) getu16(bytes, 228)/ 1000.0;
    science->Y2H[1] = (double) getu16(bytes, 460)/ 1000.0;
    science->Y2V[0] = (double) getu16(bytes, 344)/ 1000.0;
    science->Y2V[1] = (double) getu16(bytes, 576)/ 1000.0;

    // Ion admittance
    science->IonAdmittanceProbe1[0] = (double)getfloat(bytes, 48);
    science->IonAdmittanceProbe1[1] = (double)getfloat(bytes, 130);
    science->IonAdmittanceProbe2[0] = (double)getfloat(bytes, 84);
    science->IonAdmittanceProbe2[1] = (double)getfloat(bytes, 166);

    di = science->IonAdmittanceProbe1[0] - 1e-10;
    if (di < 0.0) di = 0.0;
    science->IonDensityL1aProbe1[0] = 2.0 * M_PI * 1.0e6 * di * 7600.0 * 2.67e-26 / (2 * M_PI * 0.004 * 0.004 * 1.602e-19 / 1e6); // cm^-3
    di = science->IonAdmittanceProbe1[1] - 1e-10;
    if (di < 0.0) di = 0.0;
    science->IonDensityL1aProbe1[1] = 2.0 * M_PI * 1.0e6 * di * 7600.0 * 2.67e-26 / (2 * M_PI * 0.004 * 0.004 * 1.602e-19 / 1e6); // cm^-3

    di = science->IonAdmittanceProbe2[0] - 1e-10;
    if (di < 0.0) di = 0.0;
    science->IonDensityL1aProbe2[0] = 2.0 * M_PI * 1.0e6 * di * 7600.0 * 2.67e-26 / (2 * M_PI * 0.004 * 0.004 * 1.602e-19 / 1e6); // cm^-3
    di = science->IonAdmittanceProbe2[1] - 1e-10;
    if (di < 0.0) di = 0.0;
    science->IonDensityL1aProbe2[1] = 2.0 * M_PI * 1.0e6 * di * 7600.0 * 2.67e-26 / (2 * M_PI * 0.004 * 0.004 * 1.602e-19 / 1e6); // cm^-3

    for (int i = 0; i < 32; i++)
    {
        science->ColumnSumH[0][i] = getu16(bytes, 230 + 2 * i);
        science->ColumnSumV[0][i] = getu16(bytes, 346 + 2 * i);
        science->ColumnSumH[1][i] = getu16(bytes, 462 + 2 * i);
        science->ColumnSumV[1][i] = getu16(bytes, 578 + 2 * i);
    }

    setDateTime(&(science->dateTime), pkt->DataFieldHeader);

    return;

}

void getConfigData(ConfigPacket * pkt, Config * config)
{
    uint8_t *commonBytes = pkt->TiiAuxDataCommon;
    uint8_t *auxH = pkt->TiiAuxDataH;
    uint8_t *auxV = pkt->TiiAuxDataV;

    config->agcIncrementMcpVoltage = commonBytes[0];
    config->agcIncrementShutterDutyCycle = commonBytes[1];
    config->agcEnabled = (commonBytes[2] >> 7) & 0x01;
    config->nColumnsForMomentCalculations = ((commonBytes[2] >> 1) & 0x3f) + 1;
    config->agcUpperThreshold = (commonBytes[2] & 0x01) << 15 | (commonBytes[3] << 7) | ((commonBytes[4] >> 1) & 0x7f);
    config->agcLowerThreshold = (commonBytes[4] & 0x01) << 15 | (commonBytes[5] << 7) | ((commonBytes[6] >> 1) & 0x7f);
    config->tiiMinimumColumn = ((commonBytes[6] & 0x01) << 7 | ((commonBytes[7] >> 1) & 0x7f)) + 1;
    config->tiiMaximumColumn = ((commonBytes[7] & 0x01) << 7 | ((commonBytes[8] >>1) & 0x7f)) + 1;
    config->pixelThreshold = ((commonBytes[8] & 0x01) << 11) | (commonBytes[9] << 3) | ((commonBytes[10] >> 5) & 0x07);

    config->phosphorVoltageSettingH = auxH[0];
    config->mcpVoltageSettingH = auxH[1];
    config->biasGridVoltageSettingH = auxH[2];
    config->shutterLowerPlateauVoltageSettingH = auxH[3];
    config->shutterDutyCycleH = getu16(auxH, 4);
    config->gainMapIdH = (auxH[6] >> 4) & 0x0f;

    config->phosphorVoltageSettingV = auxV[0];
    config->mcpVoltageSettingV = auxV[1];
    config->biasGridVoltageSettingV = auxV[2];
    config->shutterLowerPlateauVoltageSettingV = auxV[3];
    config->shutterDutyCycleV = getu16(auxV, 4);
    config->gainMapIdV = (auxV[6] >> 4) & 0x0f;

    setDateTime(&(config->dateTime), pkt->DataFieldHeader);

    return;

}

uint16_t getu16(uint8_t *bytes, int offset)
{
    uint16_t tmp = bytes[offset]*256 + bytes[offset+1];
    return tmp;
}

int16_t gets16(uint8_t *bytes, int offset)
{
    uint16_t tmp = bytes[offset]*256 + bytes[offset+1];
    return *(int16_t*)&tmp;
}

uint32_t getu32(uint8_t *bytes, int offset)
{
    uint32_t tmp = bytes[offset]*256*256*256 + bytes[offset+1]*256*256 + bytes[offset+2] * 256 + bytes[offset+3];
    return tmp;
}

int32_t gets32(uint8_t *bytes, int offset)
{
    uint32_t tmp = bytes[offset]*256*256*256 + bytes[offset+1]*256*256 + bytes[offset+2] * 256 + bytes[offset+3];
    return tmp;
    return *(int32_t*)&tmp;
}

float getfloat(uint8_t *bytes, int offset)
{
    uint32_t tmp = bytes[offset]*256*256*256 + bytes[offset+1]*256*256 + bytes[offset+2] * 256 + bytes[offset+3];
    return *(float*)&tmp;
}
