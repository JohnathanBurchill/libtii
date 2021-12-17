#include "isp.h"
#include "tiim.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void getImageData(FullImagePacket * fip, FullImageContinuedPacket * cip, ImageAuxData * aux, uint16_t *pixels)
{

    uint8_t * fullBytes = fip->AuxData;
    uint8_t * contBytes = cip->AuxData;

    aux->consistentImage = true;
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

    aux->SensorNumber = (fullBytes[9] >> 1) & 0x01;
    aux->GainTableId = ((fullBytes[11] & 0x01) << 3) | (fullBytes[12] >> 5);
    aux->EfiInstrumentId = (fullBytes[12] >> 1) & 0x0f;

    char efiUnit[4] = {'X', 'C', 'B', 'A'};
    aux->satellite = efiUnit[aux->EfiInstrumentId];
    aux->sensor = aux->SensorNumber ? 'V' : 'H';
    // ISP time (epoch 2000-01-01 00:00:00 UT)
    uint8_t * cds = fip->DataFieldHeader+4;
    double day = (double) (cds[0]*256 + cds[1] + 10957); // relative to 1970-01-01 00:00:00 UT
    double ms = (double) (cds[2]*256*256*256 + cds[3]*256*256 + cds[4]*256 + cds[5]);
    double us = (double) (cds[6]*256 + cds[7]);
    double secondsSince1970 = day * 86400. + ms / 1.0e3 + us / 1.0e6;
    time_t seconds = floor(secondsSince1970);
    struct tm *timeStruct = gmtime(&seconds);
    aux->year = timeStruct->tm_year + 1900;
    aux->month = timeStruct->tm_mon + 1;
    aux->day = timeStruct->tm_mday;
    aux->hour = timeStruct->tm_hour;
    aux->minute = timeStruct->tm_min;
    aux->second = timeStruct->tm_sec;
    aux->millisecond = floor((secondsSince1970 - (double)seconds)*1000);

    uint8_t contSensor = (contBytes[0] >> 7) & 0x01;
    uint8_t contGainTableId = ((contBytes[0] & 0x03) << 2) | (contBytes[1] >> 6);
    uint8_t contEfiInstrumentId = ((contBytes[1] >> 2) & 0x0f);
    // Init pixels to zeros
    memset((void*)pixels, 0, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));

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

    // Need a measurement time
    // TODO compare times as well
    if (contSensor != aux->SensorNumber || contGainTableId != aux->GainTableId || contEfiInstrumentId != aux->EfiInstrumentId)
    {
        // printf("Problem here!: %d,%d  %d,%d  %d,%d\n", aux->SensorNumber, contSensor, aux->GainTableId, contGainTableId, aux->EfiInstrumentId, contEfiInstrumentId);
        aux->consistentImage = false;
        // Return a partial image
        return;
    }
    // Got the right continued packet, finish the image
    b = 0;
    for (int i = 0; i < NUM_FULL_IMAGE_CONT_PACKET_PIXELS-1; i+=2)
    {
        pixels[p++] = (cip->PixelBytes[b] << 4) | ((cip->PixelBytes[b+1] >> 4) & 0x0f);
        pixels[p++] = ((cip->PixelBytes[b+1] & 0x0f) << 8) | cip->PixelBytes[b+2];
        b+=3;
    }
    pixels[p] = (cip->PixelBytes[b] << 4) | ((cip->PixelBytes[b+1] >> 4) & 0x0f);

}


void getImagePair(uint8_t *fullImagePackets, uint8_t *continuedPackets, long packetIndex, long numberOfPackets, ImageAuxData * aux1, uint16_t *pixels1, ImageAuxData *aux2, uint16_t *pixels2)
{

    FullImagePacket *fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;

    if (packetIndex < numberOfPackets)
    {
        fip1 = (FullImagePacket*)(fullImagePackets + packetIndex*FULL_IMAGE_PACKET_SIZE);
        cip1 = (FullImageContinuedPacket*)(continuedPackets + packetIndex*FULL_IMAGE_CONT_PACKET_SIZE);
        getImageData(fip1, cip1, aux1, pixels1);
    }
    else
    {
        memset(fip1, 0, FULL_IMAGE_PACKET_SIZE);
        memset(cip1, 0, FULL_IMAGE_CONT_PACKET_SIZE);
        aux1->EfiInstrumentId = UNIT_INVALID;
    }
    if (packetIndex + 1 < numberOfPackets)
    {
        fip2 = (FullImagePacket*)(fullImagePackets + (packetIndex+1)*FULL_IMAGE_PACKET_SIZE);
        cip2 = (FullImageContinuedPacket*)(continuedPackets + (packetIndex+1)*FULL_IMAGE_CONT_PACKET_SIZE);
        getImageData(fip2, cip2, aux2, pixels2);
    }
    else
    {
        memset(fip2, 0, FULL_IMAGE_PACKET_SIZE);
        memset(cip2, 0, FULL_IMAGE_CONT_PACKET_SIZE);
        aux2->EfiInstrumentId = UNIT_INVALID;
    }
}


int alignImages(ImageAuxData *aux1, uint16_t *pixels1, ImageAuxData *aux2, uint16_t *pixels2, bool *gotHImage, bool *gotVImage)
{
    int status = ISP_ALIGNED_IMAGE_PAIR;

    // Init to true if we have image data
    // EfiInstrumentId is 0 if image data are zeros (i.e., missing packet)
    *gotHImage = aux1->EfiInstrumentId != UNIT_INVALID; // Can be 0 when packet is missing
    *gotVImage = aux2->EfiInstrumentId != UNIT_INVALID;

    if (!(*gotHImage) && !(*gotVImage))
    {
        status = ISP_NO_IMAGE_PAIR;
    }
    else if (aux1->SensorNumber == H_SENSOR && aux2->SensorNumber == V_SENSOR)
    {
        status = ISP_ALIGNED_IMAGE_PAIR;
    }
    // Check that image 1 is an H image. If not, increase i by 1 and decrease numFullImageRecords by 1.
    // Some won't like this :)
    else if (aux1->SensorNumber == V_SENSOR)
    {
        *gotHImage = false;
        // We got a V image first. Draw an empty H image.
        // Swap H and V and zero out H pixels
        memcpy(pixels2, pixels1, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));
        memset(pixels1, FOREGROUND_COLOR, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));
        memcpy(aux2, aux1, sizeof(ImageAuxData));
        memset(aux1, 0, sizeof(ImageAuxData));
        status = ISP_V_IMAGE;
    }
    else if (aux1->SensorNumber == 0 && aux2->SensorNumber == 0)
    {
        *gotVImage = false;
        // Zero out V pixels
        memset(pixels2, FOREGROUND_COLOR, NUM_FULL_IMAGE_PIXELS * sizeof(uint16_t));
        memset(aux2, 0, sizeof(ImageAuxData));
        status = ISP_H_IMAGE;
    }
    return status;
}

int getAlignedImagePair(uint8_t *fullImagePackets, uint8_t *continuedPackets, long packetIndex, long numberOfPackets, ImageAuxData * aux1, uint16_t *pixels1, ImageAuxData *aux2, uint16_t *pixels2, bool *gotHImage, bool *gotVImage, int *imagesRead)
{
    int status = 0;
    getImagePair(fullImagePackets, continuedPackets, packetIndex, numberOfPackets, aux1, pixels1, aux2, pixels2);
    status = alignImages(aux1, pixels1, aux2, pixels2, gotHImage, gotVImage);

    if (status == ISP_ALIGNED_IMAGE_PAIR)
        *imagesRead = 2;
    else
        *imagesRead = 1;

    return status;
}
