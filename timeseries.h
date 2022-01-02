#ifndef _TIMESERIES_H
#define _TIMESERIES_H

#include "isp.h"

#include <stdlib.h>

enum TIMESERIESERROR
{
    TIME_SERIES_OK = 0,
    TIME_SERIES_MALLOC = -1
};

typedef struct LpTiiTimeSeries
{

    size_t n2Hz;
    double *lpTiiTime2Hz;
    double minTime2Hz;
    double maxTime2Hz;

    double *ionDensity1;
    double *ionDensity2;
    double *y2H;
    double *y2V;
    double *biasGridVoltageSettingH;
    double *biasGridVoltageSettingV;
    double *mcpVoltageSettingH;
    double *mcpVoltageSettingV;
    double *phosphorVoltageSettingH;
    double *phosphorVoltageSettingV;

    size_t n16Hz;
    double *lpTiiTime16Hz;
    double *x1H;
    double *y1H;
    double *faceplateCurrent;

    size_t nSweep;
    double *lpSweepTime;

    size_t nOffset;
    double *offsetTime;
    
    size_t nConfig;
    double *configTime;

} LpTiiTimeSeries;

typedef struct ImagePairTimeSeries
{
    char satellite;
    size_t nImagePairs;
    double minTime;
    double maxTime;

    // arrays
    double *time;
    bool *consistentImageH;
    int *maxValueH;

    bool *consistentImageV;
    int *maxValueV;

    uint16_t *CcdDarkCurrentH;
    double *CcdTemperatureH;
    double *FaceplateVoltageMonitorH;
    double *BiasGridVoltageMonitorH;
    double *ShutterDutyCycleH;
    double *McpVoltageMonitorH;
    double *PhosphorVoltageMonitorH;

    uint16_t *CcdDarkCurrentV;
    double *CcdTemperatureV;
    double *FaceplateVoltageMonitorV;
    double *BiasGridVoltageMonitorV;
    double *ShutterDutyCycleV;
    double *McpVoltageMonitorV;
    double *PhosphorVoltageMonitorV;

    // anomalies
    int *paCountH;
    int *maxPaValueH;
    int *cumulativePaCountH;
    int *paCumulativeFrameCountH;
    int *measlesCountH;
    int *cumulativeMeaslesCountH;
    int *paAngularSpectrumH;
    int *paAngularSpectrumMaxH;
    int *paAngularSpectrumTotalH;
    int *paAngularSpectrumCumulativeFrameCountH;

    int *paCountV;
    int *maxPaValueV;
    int *cumulativePaCountV;
    int *paCumulativeFrameCountV;
    int *measlesCountV;
    int *cumulativeMeaslesCountV;
    int *paAngularSpectrumV;
    int *paAngularSpectrumMaxV;
    int *paAngularSpectrumTotalV;
    int *paAngularSpectrumCumulativeFrameCountV;


} ImagePairTimeSeries;

int initImagePairTimeSeries(char satellite, ImagePairTimeSeries *timeSeries);

int getImagePairTimeSeries(ImagePackets *packets, ImagePair *imagePair, ImagePairTimeSeries *timeSeries, size_t numberOfImagePairs, double dayStart, double dayEnd, double max);

void freeImagePairTimeSeries(ImagePairTimeSeries * timeSeries);

void freeLpTiiTimeSeries(LpTiiTimeSeries * timeSeries);

int getLpTiiTimeSeries(SciencePackets *packets, LpTiiTimeSeries *timeSeries);


#endif // _TIMESERIES_H
