#include "isp.h"

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

    char efiUnit[3] = {'C', 'B', 'A'};
    aux->satellite = efiUnit[aux->EfiInstrumentId-1];
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


int getImagePair(uint8_t *fullImagePackets, uint8_t *continuedPackets, long packetIndex, long numberOfPackets, ImageAuxData * aux1, uint16_t *pixels1, ImageAuxData *aux2, uint16_t *pixels2)
{
    if (packetIndex > numberOfPackets-2)
        return -1;

    FullImagePacket *fip1 = (FullImagePacket*)(fullImagePackets + packetIndex*FULL_IMAGE_PACKET_SIZE);
    FullImageContinuedPacket *cip1 = (FullImageContinuedPacket*)(continuedPackets + packetIndex*FULL_IMAGE_CONT_PACKET_SIZE);
    FullImagePacket *fip2 = (FullImagePacket*)(fullImagePackets + (packetIndex+1)*FULL_IMAGE_PACKET_SIZE);
    FullImageContinuedPacket *cip2 = (FullImageContinuedPacket*)(continuedPackets + (packetIndex+1)*FULL_IMAGE_CONT_PACKET_SIZE);
    getImageData(fip1, cip1, aux1, pixels1);
    getImageData(fip2, cip2, aux2, pixels2);

    return 0;
}