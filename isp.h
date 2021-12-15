#ifndef _ISP_H
#define _ISP_H

#include <stdint.h>
#include <stdbool.h>

#define FULL_IMAGE_PACKET_SIZE 2068
#define FULL_IMAGE_CONT_PACKET_SIZE 1996

#define NUM_FULL_IMAGE_PACKET_PIXELS 1341
#define NUM_FULL_IMAGE_CONT_PACKET_PIXELS 1299
#define NUM_FULL_IMAGE_PIXELS 2640

typedef struct fullImagePacket
{
    uint8_t IspHeader[20];
    uint8_t SourcePacketHeader[6];
    uint8_t DataFieldHeader[12];
    uint8_t StructureId; // 37 bytes

    uint8_t AuxData[13];
    uint8_t MeasurementTimestamp[2];
    uint8_t PixelBytes[2012];
    uint8_t ErrorControlField[2];

} FullImagePacket;

typedef struct fullImageContinuedPacket
{
    uint8_t IspHeader[20];
    uint8_t SourcePacketHeader[6];
    uint8_t DataFieldHeader[12];
    uint8_t StructureId; // 37 bytes

    uint8_t AuxData[4];
    uint8_t MeasurementTimestamp[2];
    uint8_t PixelBytes[1949];
    uint8_t ErrorControlField[2];

} FullImageContinuedPacket;


typedef struct imageAuxData {
    uint16_t CcdDarkCurrent;
    double CcdTemperature;
    uint16_t FaceplateVoltageMonitorRaw;
    double FaceplateVoltageMonitor;
    uint16_t BiasGridVoltageMonitorRaw;
    double BiasGridVoltageMonitor;
    uint16_t ShutterDutyCycleRaw;
    double ShutterDutyCycle;
    uint16_t McpVoltageMonitorRaw;
    double McpVoltageMonitor;
    uint16_t PhosphorVoltageMonitorRaw;
    double PhosphorVoltageMonitor;
    uint8_t SensorNumber;
    uint8_t AuxReserved1;
    uint8_t AuxReserved2;
    uint8_t AuxReserved3;
    uint8_t GainTableId;
    uint8_t EfiInstrumentId;
    uint8_t AuxFiller;
    bool consistentImage;

} ImageAuxData;

void getImageData(FullImagePacket * fip, FullImageContinuedPacket *cip, ImageAuxData *aux, uint16_t *pixels);

#endif // _ISP_H
