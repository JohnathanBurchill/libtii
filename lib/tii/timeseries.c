#include "timeseries.h"
#include "isp.h"
#include "analysis.h"
#include "utility.h"

#include <stdio.h>

void initImagePairTimeSeries(ImagePairTimeSeries *ts)
{
    ts->satellite = 'X';

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

    ts->totalCountsH = NULL;
    ts->x1H = NULL;
    ts->y1H = NULL;
    ts->agcControlValueH = NULL;
    ts->totalCountsV = NULL;
    ts->x1V = NULL;
    ts->y1V = NULL;
    ts->agcControlValueV = NULL;

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

    if (ts->totalCountsH != NULL)
        free(ts->totalCountsH);
    if (ts->x1H != NULL)
        free(ts->x1H);
    if (ts->y1H != NULL)
        free(ts->y1H);
    if (ts->agcControlValueH != NULL)
        free(ts->agcControlValueH);
    if (ts->totalCountsV != NULL)
        free(ts->totalCountsV);
    if (ts->x1V != NULL)
        free(ts->x1V);
    if (ts->y1V != NULL)
        free(ts->y1V);
    if (ts->agcControlValueV != NULL)
        free(ts->agcControlValueV);
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
    if (ts->columnSumH != NULL)
        free(ts->columnSumH);
    if (ts->columnSumV != NULL)
        free(ts->columnSumV);
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
    if (ts->agcIncrementMcpVoltageConfig == NULL)
        free(ts->agcIncrementMcpVoltageConfig);
    if (ts->agcIncrementShutterDutyCycleConfig == NULL)
        free(ts->agcIncrementShutterDutyCycleConfig);
    if (ts->agcEnabledConfig == NULL)
        free(ts->agcEnabledConfig);
    if (ts->nColumnsForMomentCalculationsConfig == NULL)
        free(ts->nColumnsForMomentCalculationsConfig);
    if (ts->agcUpperThresholdConfig == NULL)
        free(ts->agcUpperThresholdConfig);
    if (ts->agcLowerThresholdConfig == NULL)
        free(ts->agcLowerThresholdConfig);
    if (ts->tiiMinimumColumnConfig == NULL)
        free(ts->tiiMinimumColumnConfig);
    if (ts->tiiMaximumColumnConfig == NULL)
        free(ts->tiiMaximumColumnConfig);
    if (ts->pixelThresholdConfig == NULL)
        free(ts->pixelThresholdConfig);
    if (ts->phosphorVoltageSettingHConfig == NULL)
        free(ts->phosphorVoltageSettingHConfig);
    if (ts->mcpVoltageSettingHConfig == NULL)
        free(ts->mcpVoltageSettingHConfig);
    if (ts->biasGridVoltageSettingHConfig == NULL)
        free(ts->biasGridVoltageSettingHConfig);
    if (ts->shutterLowerPlateauVoltageSettingHConfig == NULL)
        free(ts->shutterLowerPlateauVoltageSettingHConfig);
    if (ts->shutterDutyCycleHConfig == NULL)
        free(ts->shutterDutyCycleHConfig);
    if (ts->gainMapIdHConfig == NULL)
        free(ts->gainMapIdHConfig);
    if (ts->phosphorVoltageSettingVConfig == NULL)
        free(ts->phosphorVoltageSettingVConfig);
    if (ts->mcpVoltageSettingVConfig == NULL)
        free(ts->mcpVoltageSettingVConfig);
    if (ts->biasGridVoltageSettingVConfig == NULL)
        free(ts->biasGridVoltageSettingVConfig);
    if (ts->shutterLowerPlateauVoltageSettingVConfig == NULL)
        free(ts->shutterLowerPlateauVoltageSettingVConfig);
    if (ts->shutterDutyCycleVConfig == NULL)
        free(ts->shutterDutyCycleVConfig);
    if (ts->gainMapIdVConfig == NULL)
        free(ts->gainMapIdVConfig);

}


int getImagePairTimeSeries(char satellite, ImagePackets * imagePackets, ImagePair *imagePair, ImagePairTimeSeries *ts, size_t numberOfImagePairs, double dayStart, double dayEnd, double max)
{

    int status;

    ts->satellite = satellite;

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


        // onboard processing results
        ts->totalCountsH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->totalCountsH == NULL)
            return TIME_SERIES_MALLOC;
        ts->x1H = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->x1H == NULL)
            return TIME_SERIES_MALLOC;
        ts->y1H = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->y1H == NULL)
            return TIME_SERIES_MALLOC;
        ts->agcControlValueH = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->agcControlValueH == NULL)
            return TIME_SERIES_MALLOC;
        ts->totalCountsV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->totalCountsV == NULL)
            return TIME_SERIES_MALLOC;
        ts->x1V = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->x1V == NULL)
            return TIME_SERIES_MALLOC;
        ts->y1V = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->y1V == NULL)
            return TIME_SERIES_MALLOC;
        ts->agcControlValueV = (double *)malloc(numberOfImagePairs * sizeof(double));
        if (ts->agcControlValueV == NULL)
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

            if (status == ISP_NO_IMAGE_PAIR)
            {
                i++;
                continue;
            }
            i+=imagesRead;

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

    return TIME_SERIES_OK;
    

}

void initLpTiiTimeSeries(LpTiiTimeSeries * timeSeries)
{
    timeSeries->satellite = 'X';

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
    timeSeries->columnSumH = NULL;
    timeSeries->columnSumV = NULL;


    timeSeries->lpTiiTime16Hz = NULL;
    timeSeries->x1H = NULL;
    timeSeries->y1H = NULL;
    timeSeries->faceplateCurrent = NULL;
    timeSeries->lpSweepTime = NULL;
    timeSeries->offsetTime = NULL;

    timeSeries->configTime = NULL;
    timeSeries->agcIncrementMcpVoltageConfig = NULL;
    timeSeries->agcIncrementShutterDutyCycleConfig = NULL;
    timeSeries->agcEnabledConfig = NULL;
    timeSeries->nColumnsForMomentCalculationsConfig = NULL;
    timeSeries->agcUpperThresholdConfig = NULL;
    timeSeries->agcLowerThresholdConfig = NULL;
    timeSeries->tiiMinimumColumnConfig = NULL;
    timeSeries->tiiMaximumColumnConfig = NULL;
    timeSeries->pixelThresholdConfig = NULL;
    timeSeries->phosphorVoltageSettingHConfig = NULL;
    timeSeries->mcpVoltageSettingHConfig = NULL;
    timeSeries->biasGridVoltageSettingHConfig = NULL;
    timeSeries->shutterLowerPlateauVoltageSettingHConfig = NULL;
    timeSeries->shutterDutyCycleHConfig = NULL;
    timeSeries->gainMapIdHConfig = NULL;
    timeSeries->phosphorVoltageSettingVConfig = NULL;
    timeSeries->mcpVoltageSettingVConfig = NULL;
    timeSeries->biasGridVoltageSettingVConfig = NULL;
    timeSeries->shutterLowerPlateauVoltageSettingVConfig = NULL;
    timeSeries->shutterDutyCycleVConfig = NULL;
    timeSeries->gainMapIdVConfig = NULL;
}

int getLpTiiTimeSeries(char satellite, SciencePackets *packets, LpTiiTimeSeries * timeSeries)
{
    int status = TIME_SERIES_OK;

    timeSeries->satellite = satellite;

    LpTiiSciencePacket *pkt;
    LpTiiScience science;
    ConfigPacket *cfg;
    Config config;

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

        timeSeries->columnSumH = (uint16_t*) malloc(timeSeries->n2Hz * sizeof(uint16_t) * 32);
        if (timeSeries->columnSumH == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->columnSumV = (uint16_t*) malloc(timeSeries->n2Hz * sizeof(uint16_t) * 32);
        if (timeSeries->columnSumV == NULL)
            return TIME_SERIES_MALLOC;


        for (long i = 0; i < packets->numberOfLpTiiSciencePackets; i++)
        {
            pkt = (LpTiiSciencePacket*)(packets->lpTiiSciencePackets + i*LP_TII_SCIENCE_PACKET_SIZE);
            getLpTiiScienceData(pkt, &science);
            // TODO set accurate times
            timeSeries->lpTiiTime2Hz[2*i] = science.dateTime.secondsSince1970;
            timeSeries->lpTiiTime2Hz[2*i+1] = science.dateTime.secondsSince1970 + 0.5;
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
                for (int p = 0; p < 32; p++)
                {
                    timeSeries->columnSumH[2*32*i + 32*s + p] = science.ColumnSumH[s][p];
                    timeSeries->columnSumV[2*32*i + 32*s + p] = science.ColumnSumV[s][p];
                }
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
    if (packets->numberOfConfigPackets > 0)
    {
        timeSeries->nConfig = packets->numberOfConfigPackets;
        timeSeries->configTime = (double*) malloc(timeSeries->nConfig * sizeof(double));
        if (timeSeries->configTime == NULL)
            return TIME_SERIES_MALLOC;

        timeSeries->agcIncrementMcpVoltageConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->agcIncrementMcpVoltageConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->agcIncrementShutterDutyCycleConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->agcIncrementShutterDutyCycleConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->agcEnabledConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->agcEnabledConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->nColumnsForMomentCalculationsConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->nColumnsForMomentCalculationsConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->agcUpperThresholdConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->agcUpperThresholdConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->agcLowerThresholdConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->agcLowerThresholdConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->tiiMinimumColumnConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->tiiMinimumColumnConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->tiiMaximumColumnConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->tiiMaximumColumnConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->pixelThresholdConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->pixelThresholdConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->phosphorVoltageSettingHConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->phosphorVoltageSettingHConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->mcpVoltageSettingHConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->mcpVoltageSettingHConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->biasGridVoltageSettingHConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->biasGridVoltageSettingHConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->shutterLowerPlateauVoltageSettingHConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->shutterLowerPlateauVoltageSettingHConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->shutterDutyCycleHConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->shutterDutyCycleHConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->gainMapIdHConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->gainMapIdHConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->phosphorVoltageSettingVConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->phosphorVoltageSettingVConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->mcpVoltageSettingVConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->mcpVoltageSettingVConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->biasGridVoltageSettingVConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->biasGridVoltageSettingVConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->shutterLowerPlateauVoltageSettingVConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->shutterLowerPlateauVoltageSettingVConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->shutterDutyCycleVConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->shutterDutyCycleVConfig == NULL)
            return TIME_SERIES_MALLOC;
        timeSeries->gainMapIdVConfig = (int *) malloc(timeSeries->nConfig * sizeof(int));
        if (timeSeries->gainMapIdVConfig == NULL)
            return TIME_SERIES_MALLOC;

        for (long i = 0; i < packets->numberOfConfigPackets; i++)
        {
            cfg = (ConfigPacket*)(packets->configPackets + i*CONFIG_PACKET_SIZE);
            getConfigData(cfg, &config);
            timeSeries->configTime[i] = config.dateTime.secondsSince1970;

            timeSeries->agcIncrementMcpVoltageConfig[i] = config.agcIncrementMcpVoltage;
            timeSeries->agcIncrementShutterDutyCycleConfig[i] = config.agcIncrementShutterDutyCycle;
            timeSeries->agcEnabledConfig[i] = config.agcEnabled;
            timeSeries->nColumnsForMomentCalculationsConfig[i] = config.nColumnsForMomentCalculations;
            timeSeries->agcUpperThresholdConfig[i] = config.agcUpperThreshold;
            timeSeries->agcLowerThresholdConfig[i] = config.agcLowerThreshold;
            timeSeries->tiiMinimumColumnConfig[i] = config.tiiMinimumColumn;
            timeSeries->tiiMaximumColumnConfig[i] = config.tiiMaximumColumn;
            timeSeries->pixelThresholdConfig[i] = config.pixelThreshold;
            timeSeries->phosphorVoltageSettingHConfig[i] = config.phosphorVoltageSettingH;
            timeSeries->mcpVoltageSettingHConfig[i] = config.mcpVoltageSettingH;
            timeSeries->biasGridVoltageSettingHConfig[i] = config.biasGridVoltageSettingH;
            timeSeries->shutterLowerPlateauVoltageSettingHConfig[i] = config.shutterLowerPlateauVoltageSettingH;
            timeSeries->shutterDutyCycleHConfig[i] = config.shutterDutyCycleH;
            timeSeries->gainMapIdHConfig[i] = config.gainMapIdH;
            timeSeries->phosphorVoltageSettingVConfig[i] = config.phosphorVoltageSettingV;
            timeSeries->mcpVoltageSettingVConfig[i] = config.mcpVoltageSettingV;
            timeSeries->biasGridVoltageSettingVConfig[i] = config.biasGridVoltageSettingV;
            timeSeries->shutterLowerPlateauVoltageSettingVConfig[i] = config.shutterLowerPlateauVoltageSettingV;
            timeSeries->shutterDutyCycleVConfig[i] = config.shutterDutyCycleV;
            timeSeries->gainMapIdVConfig[i] = config.gainMapIdV;
        }

    }

    return TIME_SERIES_OK;

}

void latestConfigValues(ImagePair *imagePair, LpTiiTimeSeries *timeSeries, int *pixelThreshold, int *minCol, int *maxCol, int *nCols, bool *agcEnabled, int *agcLower, int *agcUpper)
{
    // Default values in case there are no config packets
    static size_t lastConfigIndex = 0;
    static double lastTime = 0;
    // Init to reasonable defaults
    if (pixelThreshold != NULL)
        *pixelThreshold = 0;
    if (minCol != NULL)
        *minCol = 33;
    if (maxCol != NULL)
        *maxCol = 64;
    if (nCols != NULL)
        *nCols = 32;
    if (agcEnabled != NULL)
        *agcEnabled = false;
    if (agcLower != NULL)
        *agcLower = -1;
    if (agcUpper != NULL)
        *agcUpper = -1;

    // Get config values if available. All packets must have been sorted.
    // Search from beginning if this image is older than last one
    if (imagePair->secondsSince1970 < lastTime)
        lastConfigIndex = 0;
    lastTime = imagePair->secondsSince1970;
    for (size_t i = lastConfigIndex; i < timeSeries->nConfig; i++)
    {
        lastConfigIndex = i;
        if (timeSeries->configTime[i] > imagePair->secondsSince1970)
        {
            break;
        }
    }
    if (timeSeries->nConfig > lastConfigIndex)
    {
        if (pixelThreshold != NULL)
            *pixelThreshold = timeSeries->pixelThresholdConfig[lastConfigIndex];
        if (minCol != NULL)
            *minCol = timeSeries->tiiMinimumColumnConfig[lastConfigIndex];
        if (maxCol != NULL)
            *maxCol = timeSeries->tiiMaximumColumnConfig[lastConfigIndex];
        if (nCols != NULL)
            *nCols = timeSeries->nColumnsForMomentCalculationsConfig[lastConfigIndex];
        if (agcEnabled != NULL)
            *agcEnabled = timeSeries->agcEnabledConfig[lastConfigIndex];
        if (agcLower != NULL)
            *agcLower = timeSeries->agcLowerThresholdConfig[lastConfigIndex];
        if (agcUpper != NULL)
            *agcUpper = timeSeries->agcUpperThresholdConfig[lastConfigIndex];
    }

}
