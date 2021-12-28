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
    double *ionDensity1;
    double *ionDensity2;
    double *y2H;
    double *y2V;

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

int getTimeSeries(SciencePackets *packets, LpTiiTimeSeries *timeSeries);

#endif // _TIMESERIES_H
