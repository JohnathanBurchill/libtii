#ifndef _ISP_H
#define _ISP_H

#include <stdint.h>
#include <stdbool.h>

#define FULL_IMAGE_PACKET_SIZE 2068
#define FULL_IMAGE_CONT_PACKET_SIZE 1996
#define LP_TII_SCIENCE_PACKET_SIZE 724
#define LP_SWEEP_PACKET_SIZE 1108
#define LP_OFFSET_PACKET_SIZE 720
#define CONFIG_PACKET_SIZE 116

#define NUM_FULL_IMAGE_PACKET_PIXELS 1341
#define NUM_FULL_IMAGE_CONT_PACKET_PIXELS 1299
#define TII_ROWS 66
#define TII_COLS 40
#define NUM_FULL_IMAGE_PIXELS 2640

typedef struct imagePackets
{
    uint8_t *fullImagePackets;
    uint8_t *continuedPackets;
    long numberOfImages;

} ImagePackets;

typedef struct SciencePackets
{
    uint8_t *lpTiiSciencePackets;
    long numberOfLpTiiSciencePackets;
    uint8_t *lpSweepPackets;
    long numberOfLpSweepPackets;
    uint8_t *configPackets;
    long numberOfConfigPackets;
    uint8_t *offsetPackets;
    long numberOfOffsetPackets;
} SciencePackets;


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

typedef struct LpTiiSciencePacket
{
    uint8_t IspHeader[20];
    uint8_t SourcePacketHeader[6];
    uint8_t DataFieldHeader[12];
    uint8_t StructureId; // 37 bytes

    uint8_t AuxData[4];
    uint8_t AuxDataH[8];
    uint8_t AuxDataV[8];
    uint8_t LpAuxData[1];
    uint8_t MeasurementTimestamp[2];
    uint8_t LpTiiData[660];    
    uint8_t ErrorControlField[2];

} LpTiiSciencePacket;

typedef struct LpSweepPacket
{
    uint8_t IspHeader[20];
    uint8_t SourcePacketHeader[6];
    uint8_t DataFieldHeader[12];
    uint8_t StructureId; // 37 bytes

    uint8_t AuxData[25];
    uint8_t MeasurementTimestamp[2];
    uint8_t LpSweepData[1040];
    uint8_t ErrorControlField[2];

} LpSweepPacket;

typedef struct LpOffsetPacket
{
    uint8_t IspHeader[20];
    uint8_t SourcePacketHeader[6];
    uint8_t DataFieldHeader[12];
    uint8_t StructureId; // 37 bytes

    uint8_t AuxData[10];
    uint8_t MeasurementTimestamp[2];
    uint8_t LpTiiData[667];
    uint8_t ErrorControlField[2];

} LpOffsetPacket;

typedef struct ConfigPacket
{
    uint8_t IspHeader[20];
    uint8_t SourcePacketHeader[6];
    uint8_t DataFieldHeader[12];
    uint8_t StructureId;

    uint8_t TiiAuxDataCommon[12];
    uint8_t TiiAuxDataH[9];
    uint8_t TiiAuxDataV[9];
    uint8_t LpAuxData[43];
    uint8_t MeasurementTimestamp[2];
    uint8_t ErrorControlField[2];

} ConfigPacket;

typedef struct ispDateTime {
    double secondsSince1970;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
} IspDateTime;

typedef struct imageAuxData {
    char satellite;
    char sensor;
    IspDateTime dateTime;
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

typedef struct imagePair
{
    ImageAuxData *auxH;
    uint16_t *pixelsH;
    bool gotImageH;
    ImageAuxData *auxV;
    uint16_t *pixelsV;
    bool gotImageV;
} ImagePair;

typedef struct LpTiiScience
{
    char satellite;
    IspDateTime dateTime;

    uint16_t FaceplateVoltageMonitorRaw;
    double FaceplateVoltageMonitor;

    uint16_t CcdDarkCurrentH;
    double CcdTemperatureH;
    uint16_t HvpsTemperatureMonitorRawH;
    double HvpsTemperatureH;
    uint8_t BiasGridVoltageSettingRawH;
    double BiasGridVoltageSettingH;
    uint8_t McpVoltageSettingRawH;
    double McpVoltageSettingH;
    uint8_t PhosphorVoltageSettingRawH;
    double PhosphorVoltageSettingH;
    uint16_t ShutterDutyCycleRawH;
    double ShutterDutyCycleH;

    uint16_t CcdDarkCurrentV;
    double CcdTemperatureV;
    uint16_t HvpsTemperatureMonitorRawV;
    double HvpsTemperatureV;
    uint8_t BiasGridVoltageSettingRawV;
    double BiasGridVoltageSettingV;
    uint8_t McpVoltageSettingRawV;
    double McpVoltageSettingV;
    uint8_t PhosphorVoltageSettingRawV;
    double PhosphorVoltageSettingV;
    uint16_t ShutterDutyCycleRawV;
    double ShutterDutyCycleV;

    uint8_t LpInstrumentId;

    int16_t faceplateCurrentRaw[16];

    double IonAdmittanceProbe1[2]; // 2 samples
    double IonAdmittanceProbe2[2];

    double IonDensityL1aProbe1[2];
    double IonDensityL1aProbe2[2];

    double X1H[16];
    double Y1H[16];
    double X1V[16];
    double Y1V[16];
    double Y2H[2];
    double Y2V[2];

    uint16_t ColumnSumH[2][32];
    uint16_t ColumnSumV[2][32];

    double Y18MB[2][8];

    uint16_t Y1SumAllColumnsH[2];
    uint16_t Y1SumAllColumnsV[2];

} LpTiiScience;

typedef struct LpSweep {
    char satellite;
    IspDateTime dateTime;

} LpSweep;

typedef struct Config {
    char satellite;
    IspDateTime dateTime;


} Config;



enum EFI_UNIT {
    UNIT_INVALID = 0,
    UNIT_EFI_C = 1,
    UNIT_EFI_B = 2,
    UNIT_EFI_A = 3
};

enum EFI_MODE {
    EFI_MODE_INVALID = 0,
    EFI_MODE_NORMAL = 1,
    EFI_MODE_TIICAL = 2
};

enum TII_IMAGE {
    H_SENSOR = 0,
    V_SENSOR = 1
};

enum ISP_TYPE {
    ISP_TYPE_FULL_IMAGE = 12,
    ISP_TYPE_FULL_IMAGE_CONTINUED = 15,
    ISP_TYPE_LPTII = 13
};

enum ISP_STATUS
{
    ISP_ALIGNED_IMAGE_PAIR = 0,
    ISP_NO_IMAGE_PAIR = 1,
    ISP_H_IMAGE = 2,
    ISP_V_IMAGE = 3,
    ISP_PACKET_INDEX_TOO_LARGE
};

void getImageData(FullImagePacket * fip, FullImageContinuedPacket *cip, ImageAuxData *aux, uint16_t *pixels);

void getImagePair(ImagePackets *imagePackets, long packetIndex, ImagePair *imagePair);

int alignImages(ImagePair *imagePair);

int getAlignedImagePair(ImagePackets *imagePackets, long packetIndex, ImagePair *imagePair, int *imagesRead);

int getFirstImagePair(ImagePackets *imagePackets, ImagePair *imagePair);
int getLastImagePair(ImagePackets *imagePackets, ImagePair *imagePair);

IspDateTime * getIspDateTime(ImagePair *imagePair);
char getSatellite(ImagePair *imagePair);

void initializeImagePair(ImagePair *imagePair, ImageAuxData *auxH, uint16_t *pixelsH, ImageAuxData *auxV, uint16_t *pixelsV);

void setDateTime(IspDateTime * dateTime, uint8_t *dataFieldHeader);

void getLpTiiScienceData(LpTiiSciencePacket * pkt, LpTiiScience * science);

uint16_t getu16(uint8_t *bytes, int offset);
int16_t gets16(uint8_t *bytes, int offset);
uint32_t getu32(uint8_t *bytes, int offset);
int32_t gets32(uint8_t *bytes, int offset);
float getfloat(uint8_t *bytes, int offset);

#endif // _ISP_H
