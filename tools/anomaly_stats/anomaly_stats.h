#ifndef _ANOMALY_STATS_H
#define _ANOMALY_STATS_H

#include "isp.h"

#include <stdbool.h>
#include <stdint.h>

#define CLASSIC_WING_ANOMALY_Y2_THRESHOLD 3


#define MOMENT_MIN_X 10
#define MOMENT_MAX_X 30
#define MOMENT_MIN_Y 20
#define MOMENT_MAX_Y 45
#define Y2_BOX_HALF_WIDTH 7
#define Y2_BOX_HALF_HEIGHT 7
#define BIFURCATION_ANALYSIS_DY 3 // examine above and below y1 by this amount
#define BIFURCATION_ANALYSIS_WIDTH 6
#define BIFURCATION_MINIMUM_PEAK_VALUE 400 // Analyze only images with sufficient signal in the O+ region


typedef struct ImageAnomalies {

    bool peripheralAnomaly;
    bool upperAngelsWingAnomaly;
    bool lowerAngelsWingAnomaly;
    bool classicWingAnomaly;
    bool bifurcationAnomaly;
    bool ringAnomaly;
    bool measlesAnomaly; 

} ImageAnomalies;

void analyzeRawImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies);

void analyzeGainCorrectedImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies);


void initializeAnomalyData(ImageAnomalies *anomalies);

bool scienceMode(ImageAuxData *aux);

void statsusage(const char * name);

#endif // _ANOMALY_STATS_H
