#include "timeseries.h"
#include "isp.h"
#include "analysis.h"
#include "utility.h"

#include <stdio.h>

int initImagePairTimeSeries(char satellite, ImagePairTimeSeries *ts)
{
    ts->satellite = satellite;

    ts->nImagePairs = 0;
    ts->minTime = 0.0;
    ts->maxTime = 0.0;

    ts->time = NULL;
    ts->consistentImageH = NULL;
    ts->maxValueH = NULL;
    ts->consistentImageV = NULL;
    ts->maxValueV = NULL;
    ts->CcdDarkCurrentH = NULL;
    ts->CcdTemperatureH = NULL;
    ts->FaceplateVoltageMonitorH = NULL;
    ts->BiasGridVoltageMonitorH = NULL;
    ts->ShutterDutyCycleH = NULL;
    ts->McpVoltageMonitorH = NULL;
    ts->PhosphorVoltageMonitorH = NULL;

    ts->CcdDarkCurrentV = NULL;
    ts->CcdTemperatureV = NULL;
    ts->FaceplateVoltageMonitorV = NULL;
    ts->BiasGridVoltageMonitorV = NULL;
    ts->ShutterDutyCycleV = NULL;
    ts->McpVoltageMonitorV = NULL;
    ts->PhosphorVoltageMonitorV = NULL;
    ts->paCountH = NULL;
    ts->maxPaValueH = NULL;
    ts->cumulativePaCountH = NULL;
    ts->paCumulativeFrameCountH = NULL;
    ts->measlesCountH = NULL;
    ts->cumulativeMeaslesCountH = NULL;
    ts->paAngularSpectrumH = NULL;
    ts->paAngularSpectrumMaxH = NULL;
    ts->paAngularSpectrumTotalH = NULL;
    ts->paAngularSpectrumCumulativeFrameCountH = NULL;

    ts->paCountV = NULL;
    ts->maxPaValueV = NULL;
    ts->cumulativePaCountV = NULL;
    ts->paCumulativeFrameCountV = NULL;
    ts->measlesCountV = NULL;
    ts->cumulativeMeaslesCountV = NULL;
    ts->paAngularSpectrumV = NULL;
    ts->paAngularSpectrumMaxV = NULL;
    ts->paAngularSpectrumTotalV = NULL;
    ts->paAngularSpectrumCumulativeFrameCountV = NULL;

}

void freeImagePairTimeSeries(ImagePairTimeSeries * ts)
{
    if (ts->time != NULL)
        free(ts->time);
    if (ts->consistentImageH != NULL)
        free(ts->consistentImageH);
    if (ts->maxValueH != NULL)
        free(ts->maxValueH);
    if (ts->consistentImageV != NULL)
        free(ts->consistentImageV);
    if (ts->maxValueV != NULL)
        free(ts->maxValueV);
    if (ts->CcdDarkCurrentH != NULL)
        free(ts->CcdDarkCurrentH);
    if (ts->CcdTemperatureH != NULL)
        free(ts->CcdTemperatureH);
    if (ts->FaceplateVoltageMonitorH != NULL)
        free(ts->FaceplateVoltageMonitorH);
    if (ts->BiasGridVoltageMonitorH != NULL)
        free(ts->BiasGridVoltageMonitorH);
    if (ts->ShutterDutyCycleH != NULL)
        free(ts->ShutterDutyCycleH);
    if (ts->McpVoltageMonitorH != NULL)
        free(ts->McpVoltageMonitorH);
    if (ts->PhosphorVoltageMonitorH != NULL)
        free(ts->PhosphorVoltageMonitorH);
    if (ts->CcdDarkCurrentV != NULL)
        free(ts->CcdDarkCurrentV);
    if (ts->CcdTemperatureV != NULL)
        free(ts->CcdTemperatureV);
    if (ts->FaceplateVoltageMonitorV != NULL)
        free(ts->FaceplateVoltageMonitorV);
    if (ts->BiasGridVoltageMonitorV != NULL)
        free(ts->BiasGridVoltageMonitorV);
    if (ts->ShutterDutyCycleV != NULL)
        free(ts->ShutterDutyCycleV);
    if (ts->McpVoltageMonitorV != NULL)
        free(ts->McpVoltageMonitorV);
    if (ts->PhosphorVoltageMonitorV != NULL)
        free(ts->PhosphorVoltageMonitorV);
    if (ts->paCountH != NULL)
        free(ts->paCountH);
    if (ts->maxPaValueH != NULL)
        free(ts->maxPaValueH);
    if (ts->cumulativePaCountH != NULL)
        free(ts->cumulativePaCountH);
    if (ts->paCumulativeFrameCountH != NULL)
        free(ts->paCumulativeFrameCountH);
    if (ts->measlesCountH != NULL)
        free(ts->measlesCountH);
    if (ts->cumulativeMeaslesCountH != NULL)
        free(ts->cumulativeMeaslesCountH);
    if (ts->paAngularSpectrumH != NULL)
        free(ts->paAngularSpectrumH);
    if (ts->paAngularSpectrumMaxH != NULL)
        free(ts->paAngularSpectrumMaxH);
    if (ts->paAngularSpectrumTotalH != NULL)
        free(ts->paAngularSpectrumTotalH);
    if (ts->paAngularSpectrumCumulativeFrameCountH != NULL)
        free(ts->paAngularSpectrumCumulativeFrameCountH);
    if (ts->paCountV != NULL)
        free(ts->paCountV);
    if (ts->maxPaValueV != NULL)
        free(ts->maxPaValueV);
    if (ts->cumulativePaCountV != NULL)
        free(ts->cumulativePaCountV);
    if (ts->paCumulativeFrameCountV != NULL)
        free(ts->paCumulativeFrameCountV);
    if (ts->measlesCountV != NULL)
        free(ts->measlesCountV);
    if (ts->cumulativeMeaslesCountV != NULL)
        free(ts->cumulativeMeaslesCountV);
    if (ts->paAngularSpectrumV != NULL)
        free(ts->paAngularSpectrumV);
    if (ts->paAngularSpectrumMaxV != NULL)
        free(ts->paAngularSpectrumMaxV);
    if (ts->paAngularSpectrumTotalV != NULL)
        free(ts->paAngularSpectrumTotalV);
    if (ts->paAngularSpectrumCumulativeFrameCountV != NULL)
        free(ts->paAngularSpectrumCumulativeFrameCountV);

}

void freeLpTiiTimeSeries(LpTiiTimeSeries * ts)
{
    if (ts->lpTiiTime2Hz != NULL)
        free(ts->lpTiiTime2Hz);
    if (ts->ionDensity1 != NULL)
        free(ts->ionDensity1);
    if (ts->ionDensity2 != NULL)
        free(ts->ionDensity2);
    if (ts->y2H != NULL)
        free(ts->y2H);
    if (ts->y2V != NULL)
        free(ts->y2V);
    if (ts->biasGridVoltageSettingH != NULL)
        free(ts->biasGridVoltageSettingH);
    if (ts->biasGridVoltageSettingV != NULL)
        free(ts->biasGridVoltageSettingV);
    if (ts->mcpVoltageSettingH != NULL)
        free(ts->mcpVoltageSettingH);
    if (ts->mcpVoltageSettingV != NULL)
        free(ts->mcpVoltageSettingV);
    if (ts->phosphorVoltageSettingH != NULL)
        free(ts->phosphorVoltageSettingH);
    if (ts->phosphorVoltageSettingV != NULL)
        free(ts->phosphorVoltageSettingV);
    if (ts->lpTiiTime16Hz != NULL)
        free(ts->lpTiiTime16Hz);
    if (ts->x1H != NULL)
        free(ts->x1H);
    if (ts->y1H != NULL)
        free(ts->y1H);
    if (ts->faceplateCurrent != NULL)
        free(ts->faceplateCurrent);
    if (ts->lpSweepTime != NULL)
        free(ts->lpSweepTime);
    if (ts->offsetTime != NULL)
        free(ts->offsetTime);
    if (ts->configTime != NULL)
        free(ts->configTime);

}


int getImagePairTimeSeries(ImagePackets * imagePackets, ImagePair *imagePair, ImagePairTimeSeries *ts, size_t numberOfImagePairs, double dayStart, double dayEnd, double max)
{

    int status;

    // Allocate heap
    ts->nImagePairs = numberOfImagePairs;
    if (numberOfImagePairs > 0)
    {
        ts->time = (double*) malloc(numberOfImagePairs * sizeof(double));
        if (ts->time == NULL)
            return TIME_SERIES_MALLOC;
        ts->consistentImageH = (bool *) malloc(numberOfImagePairs * sizeof(bool));
        if (ts->consistentImageH == NULL)
            return TIME_SERIES_MALLOC;
        ts->maxValueH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->maxValueH == NULL)
            return TIME_SERIES_MALLOC;

        ts->consistentImageV = (bool *) malloc(numberOfImagePairs * sizeof(bool));
        if (ts->consistentImageV == NULL)
            return TIME_SERIES_MALLOC;
        ts->maxValueV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->maxValueV == NULL)
            return TIME_SERIES_MALLOC;

        ts->CcdDarkCurrentH = (uint16_t*) malloc(numberOfImagePairs * sizeof(uint16_t));
        if (ts->CcdDarkCurrentH == NULL)
            return TIME_SERIES_MALLOC;
        ts->CcdTemperatureH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->CcdTemperatureH == NULL)
            return TIME_SERIES_MALLOC;
        ts->FaceplateVoltageMonitorH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->FaceplateVoltageMonitorH == NULL)
            return TIME_SERIES_MALLOC;
        ts->BiasGridVoltageMonitorH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->BiasGridVoltageMonitorH == NULL)
            return TIME_SERIES_MALLOC;
        ts->ShutterDutyCycleH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->ShutterDutyCycleH == NULL)
            return TIME_SERIES_MALLOC;
        ts->McpVoltageMonitorH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->McpVoltageMonitorH == NULL)
            return TIME_SERIES_MALLOC;
        ts->PhosphorVoltageMonitorH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->PhosphorVoltageMonitorH == NULL)
            return TIME_SERIES_MALLOC;

        ts->CcdDarkCurrentV = (uint16_t*) malloc(numberOfImagePairs * sizeof(uint16_t));
        if (ts->CcdDarkCurrentV == NULL)
            return TIME_SERIES_MALLOC;
        ts->CcdTemperatureV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->CcdTemperatureV == NULL)
            return TIME_SERIES_MALLOC;
        ts->FaceplateVoltageMonitorV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->FaceplateVoltageMonitorV == NULL)
            return TIME_SERIES_MALLOC;
        ts->BiasGridVoltageMonitorV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->BiasGridVoltageMonitorV == NULL)
            return TIME_SERIES_MALLOC;
        ts->ShutterDutyCycleV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->ShutterDutyCycleV == NULL)
            return TIME_SERIES_MALLOC;
        ts->McpVoltageMonitorV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->McpVoltageMonitorV == NULL)
            return TIME_SERIES_MALLOC;
        ts->PhosphorVoltageMonitorV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->PhosphorVoltageMonitorV == NULL)
            return TIME_SERIES_MALLOC;

        // anomalies
        ts->paCountH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->paCountH == NULL)
            return TIME_SERIES_MALLOC;
        ts->maxPaValueH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->maxPaValueH == NULL)
            return TIME_SERIES_MALLOC;
        ts->cumulativePaCountH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->cumulativePaCountH == NULL)
            return TIME_SERIES_MALLOC;
        ts->paCumulativeFrameCountH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->paCumulativeFrameCountH == NULL)
            return TIME_SERIES_MALLOC;
        ts->measlesCountH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->measlesCountH == NULL)
            return TIME_SERIES_MALLOC;
        ts->cumulativeMeaslesCountH = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->cumulativeMeaslesCountH == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumH = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumH == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumMaxH = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumMaxH == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumTotalH = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumTotalH == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumCumulativeFrameCountH = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumCumulativeFrameCountH == NULL)
            return TIME_SERIES_MALLOC;

        ts->paCountV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->paCountV == NULL)
            return TIME_SERIES_MALLOC;
        ts->maxPaValueV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->maxPaValueV == NULL)
            return TIME_SERIES_MALLOC;
        ts->cumulativePaCountV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->cumulativePaCountV == NULL)
            return TIME_SERIES_MALLOC;
        ts->paCumulativeFrameCountV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->paCumulativeFrameCountV == NULL)
            return TIME_SERIES_MALLOC;
        ts->measlesCountV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->measlesCountV == NULL)
            return TIME_SERIES_MALLOC;
        ts->cumulativeMeaslesCountV = (int *) malloc(numberOfImagePairs * sizeof(int));
        if (ts->cumulativeMeaslesCountV == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumV = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumV == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumMaxV = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumMaxV == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumTotalV = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumTotalV == NULL)
            return TIME_SERIES_MALLOC;
        ts->paAngularSpectrumCumulativeFrameCountV = (int *) malloc(PA_ANGULAR_NUM_BINS * numberOfImagePairs * sizeof(int));
        if (ts->paAngularSpectrumCumulativeFrameCountV == NULL)
            return TIME_SERIES_MALLOC;


        int imagesRead = 0;

        ImageStats statsH, statsV;
        initializeImageStats(&statsH);
        initializeImageStats(&statsV);

        size_t index = 0;
        int paBin = 0;
        ts->minTime = 1e20;
        ts->maxTime = -1e20;

        for (int i = 0; i < imagePackets->numberOfImages-1;)
        {
            status = getAlignedImagePair(imagePackets, i, imagePair, &imagesRead);

            i+=imagesRead;
            if (status == ISP_NO_IMAGE_PAIR)
                continue;

            if (ignoreTime(imagePair->secondsSince1970, dayStart, dayEnd))
                continue;

            //analyze imagery
            analyzeImage(imagePair->pixelsH, imagePair->gotImageH, max, &statsH);
            analyzeImage(imagePair->pixelsV, imagePair->gotImageV, max, &statsV);

            // populate time series
            ts->time[index] = imagePair->secondsSince1970;
            if (ts->time[index] < ts->minTime) ts->minTime = ts->time[index];
            if (ts->time[index] > ts->maxTime) ts->maxTime = ts->time[index];

            ts->consistentImageH[index] = imagePair->gotImageH;
            ts->maxValueH[index] = statsH.maxValue;

            ts->consistentImageV[index] = imagePair->gotImageV;
            ts->maxValueV[index] = statsV.maxValue;

            ts->CcdDarkCurrentH[index] = imagePair->auxH->CcdDarkCurrent;
            ts->CcdTemperatureH[index] = imagePair->auxH->CcdTemperature;
            ts->FaceplateVoltageMonitorH[index] = imagePair->auxH->FaceplateVoltageMonitor;
            ts->BiasGridVoltageMonitorH[index] = imagePair->auxH->BiasGridVoltageMonitor;
            ts->ShutterDutyCycleH[index] = imagePair->auxH->ShutterDutyCycle;
            ts->McpVoltageMonitorH[index] = imagePair->auxH->McpVoltageMonitor;
            ts->PhosphorVoltageMonitorH[index] = imagePair->auxH->PhosphorVoltageMonitor;

            ts->CcdDarkCurrentV[index] = imagePair->auxV->CcdDarkCurrent;
            ts->CcdTemperatureV[index] = imagePair->auxV->CcdTemperature;
            ts->FaceplateVoltageMonitorV[index] = imagePair->auxV->FaceplateVoltageMonitor;
            ts->BiasGridVoltageMonitorV[index] = imagePair->auxV->BiasGridVoltageMonitor;
            ts->ShutterDutyCycleV[index] = imagePair->auxV->ShutterDutyCycle;
            ts->McpVoltageMonitorV[index] = imagePair->auxV->McpVoltageMonitor;
            ts->PhosphorVoltageMonitorV[index] = imagePair->auxV->PhosphorVoltageMonitor;

            // anomalies
            ts->paCountH[index] = statsH.paCount;
            ts->maxPaValueH[index] = statsH.maxPaValue;
            ts->cumulativePaCountH[index] = statsH.cumulativePaCount;
            ts->paCumulativeFrameCountH[index] = statsH.paCumulativeFrameCount;
            ts->measlesCountH[index] = statsH.measlesCount;
            ts->cumulativeMeaslesCountH[index] = statsH.cumulativeMeaslesCount;

            ts->paCountV[index] = statsV.paCount;
            ts->maxPaValueV[index] = statsV.maxPaValue;
            ts->cumulativePaCountV[index] = statsV.cumulativePaCount;
            ts->paCumulativeFrameCountV[index] = statsV.paCumulativeFrameCount;
            ts->measlesCountV[index] = statsV.measlesCount;
            ts->cumulativeMeaslesCountV[index] = statsV.cumulativeMeaslesCount;

            for (int b = 0; b < PA_ANGULAR_NUM_BINS; b++)
            {
                paBin = index*PA_ANGULAR_NUM_BINS + b;

                ts->paAngularSpectrumH[paBin] = statsH.paAngularSpectrum[b];
                ts->paAngularSpectrumMaxH[paBin] = statsH.paAngularSpectrumMax[b];
                ts->paAngularSpectrumTotalH[paBin] = statsH.paAngularSpectrumTotal[b];
                ts->paAngularSpectrumCumulativeFrameCountH[paBin] = statsH.paAngularSpectrumCumulativeFrameCount[b];

                ts->paAngularSpectrumV[paBin] = statsV.paAngularSpectrum[b];
                ts->paAngularSpectrumMaxV[paBin] = statsV.paAngularSpectrumMax[b];
                ts->paAngularSpectrumTotalV[paBin] = statsV.paAngularSpectrumTotal[b];
                ts->paAngularSpectrumCumulativeFrameCountV[paBin] = statsV.paAngularSpectrumCumulativeFrameCount[b];
            }

            index++;

        }

    
    }
    

}

void initLpTiiTimeSeries(LpTiiTimeSeries * timeSeries)
{
    timeSeries->n2Hz = 0;
    timeSeries->n16Hz = 0;
    timeSeries->nSweep = 0;
    timeSeries->nOffset = 0;
    timeSeries->nConfig = 0;

    timeSeries->lpTiiTime2Hz = NULL;
    timeSeries->ionDensity1 = NULL;
    timeSeries->ionDensity2 = NULL;
    timeSeries->y2H = NULL;
    timeSeries->y2V = NULL;
    timeSeries->biasGridVoltageSettingH = NULL;
    timeSeries->biasGridVoltageSettingV = NULL;
    timeSeries->mcpVoltageSettingH = NULL;
    timeSeries->mcpVoltageSettingV = NULL;
    timeSeries->phosphorVoltageSettingH = NULL;
    timeSeries->phosphorVoltageSettingV = NULL;


    timeSeries->lpTiiTime16Hz = NULL;
    timeSeries->x1H = NULL;
    timeSeries->y1H = NULL;
    timeSeries->faceplateCurrent = NULL;
    timeSeries->lpSweepTime = NULL;
    timeSeries->offsetTime = NULL;
    timeSeries->configTime = NULL;

}

int getLpTiiTimeSeries(SciencePackets *packets, LpTiiTimeSeries * timeSeries)
{
    int status = TIME_SERIES_OK;

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
