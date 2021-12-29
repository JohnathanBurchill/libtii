#include "timeseries.h"
#include "isp.h"

#include <stdio.h>


int getTimeSeries(SciencePackets *packets, LpTiiTimeSeries * timeSeries)
{
    int status = TIME_SERIES_OK;

    timeSeries->n2Hz = 0;
    timeSeries->n16Hz = 0;
    timeSeries->nSweep = 0;
    timeSeries->nOffset = 0;
    timeSeries->nConfig = 0;

    timeSeries->lpTiiTime16Hz = NULL;
    timeSeries->x1H = NULL;
    timeSeries->y1H = NULL;
    timeSeries->faceplateCurrent = NULL;
    timeSeries->lpSweepTime = NULL;
    timeSeries->offsetTime = NULL;
    timeSeries->configTime = NULL;

    LpTiiSciencePacket *pkt;
    LpTiiScience science;

    double minTime = 1e20;
    double maxTime = -1;

    // LP&TII Science
    if (packets->numberOfLpTiiSciencePackets > 0)
    {
        // 2 Hz
        timeSeries->n2Hz = 2 * packets->numberOfLpTiiSciencePackets;
        timeSeries->lpTiiTime2Hz = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->lpTiiTime2Hz == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->ionDensity1 = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->ionDensity1 == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->ionDensity2 = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->ionDensity2 == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->y2H = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->y2H == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->y2V = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->y2V == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->biasGridVoltageSettingH = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->biasGridVoltageSettingH == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->biasGridVoltageSettingV= (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->biasGridVoltageSettingV == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->mcpVoltageSettingH = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->mcpVoltageSettingH == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->mcpVoltageSettingV = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->mcpVoltageSettingV == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->phosphorVoltageSettingH = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->phosphorVoltageSettingH == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->phosphorVoltageSettingV = (double*) malloc(timeSeries->n2Hz * sizeof(double));
        if (timeSeries->phosphorVoltageSettingV == NULL)
            return TIME_SERIES_MALLOC;



        for (long i = 0; i < packets->numberOfLpTiiSciencePackets; i++)
        {
            pkt = (LpTiiSciencePacket*)(packets->lpTiiSciencePackets + i*LP_TII_SCIENCE_PACKET_SIZE);
            getLpTiiScienceData(pkt, &science);
            // TODO set accurate times
            timeSeries->lpTiiTime2Hz[2*i] = science.dateTime.secondsSince1970;
            timeSeries->lpTiiTime2Hz[2*i+1] = timeSeries->lpTiiTime2Hz[i] + 0.5;
            if (timeSeries->lpTiiTime2Hz[2*i] > maxTime) maxTime = timeSeries->lpTiiTime2Hz[2*i] + 0.5;
            if (timeSeries->lpTiiTime2Hz[2*i] < minTime) minTime = timeSeries->lpTiiTime2Hz[2*i];
            for (int s = 0; s < 2; s++)
            {
                timeSeries->ionDensity1[2*i+s] = science.IonDensityL1aProbe1[s];
                timeSeries->ionDensity2[2*i+s] = science.IonDensityL1aProbe2[s];
                timeSeries->y2H[2*i+s] = science.Y2H[s];
                timeSeries->y2V[2*i+s] = science.Y2V[s];
                timeSeries->biasGridVoltageSettingH[2*i + s] = science.BiasGridVoltageSettingH;
                timeSeries->biasGridVoltageSettingV[2*i + s] = science.BiasGridVoltageSettingV;
                timeSeries->mcpVoltageSettingH[2*i + s] = science.McpVoltageSettingH;
                timeSeries->mcpVoltageSettingV[2*i + s] = science.McpVoltageSettingV;
                timeSeries->phosphorVoltageSettingH[2*i + s] = science.PhosphorVoltageSettingH;
                timeSeries->phosphorVoltageSettingV[2*i + s] = science.PhosphorVoltageSettingV;
            }            
        }
        timeSeries->minTime2Hz = minTime;
        timeSeries->maxTime2Hz = maxTime;

        // 16 Hz
        timeSeries->n16Hz = 16 * packets->numberOfLpTiiSciencePackets;
        timeSeries->lpTiiTime16Hz = (double*) malloc(timeSeries->n16Hz * sizeof(double));
        if (timeSeries->lpTiiTime16Hz == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->x1H = (double*) malloc(timeSeries->n16Hz * sizeof(double));
        if (timeSeries->x1H == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->y1H = (double*) malloc(timeSeries->n16Hz * sizeof(double));
        if (timeSeries->y1H == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->faceplateCurrent = (double*) malloc(timeSeries->n16Hz * sizeof(double));
        if (timeSeries->faceplateCurrent == NULL)
            return TIME_SERIES_MALLOC;



    }
    else
    {
        timeSeries->lpTiiTime2Hz = NULL;
        timeSeries->ionDensity1 = NULL;
        timeSeries->ionDensity2 = NULL;
        timeSeries->y2H = NULL;
        timeSeries->y2V = NULL;
        timeSeries->minTime2Hz = 0.0;
        timeSeries->maxTime2Hz = 0.0;
    }

    return TIME_SERIES_OK;

}
