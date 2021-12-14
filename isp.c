#include "isp.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

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
        pixels[p] = (fip->PixelBytes[b] << 4) | ((fip->PixelBytes[b+1] >> 4) & 0x0f);
        pixels[p+1] = (fip->PixelBytes[b+1] << 8) | fip->PixelBytes[b+2];
        b+=3;
        p+=2;
    }
    pixels[p] = (fip->PixelBytes[b] << 4) | ((fip->PixelBytes[b+1] >> 4) & 0x0f);

    // Need a measurement time
    // TODO compare times as well
    if (contSensor != aux->SensorNumber || contGainTableId != aux->GainTableId || contEfiInstrumentId != aux->EfiInstrumentId)
    {
        printf("Problem here!: %d,%d  %d,%d  %d,%d\n", aux->SensorNumber, contSensor, aux->GainTableId, contGainTableId, aux->EfiInstrumentId, contEfiInstrumentId);
        aux->consistentImage = false;
        // Return a partial image
        return;
    }
    // Got the right continued packet, finish the image
    b = 0;
    for (int i = 0; i < NUM_FULL_IMAGE_CONT_PACKET_PIXELS-1; i+=2)
    {
        pixels[p] = (cip->PixelBytes[b] << 4) | ((cip->PixelBytes[b+1] >> 4) & 0x0f);
        pixels[p+1] = (cip->PixelBytes[b+1] << 8) | cip->PixelBytes[b+2];
        b+=3;
        p+=2;
    }
    pixels[p] = (cip->PixelBytes[b] << 4) | ((cip->PixelBytes[b+1] >> 4) & 0x0f);

}
