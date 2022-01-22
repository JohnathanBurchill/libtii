#ifndef _ANOMALY_STATS_H
#define _ANOMALY_STATS_H

#include "isp.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct ImageAnomalies {

    bool peripheralAnomaly;
    bool upperAngelsWingAnomaly;
    bool lowerAngelsWingAnomaly;
    bool classicWingAnomaly;
    bool bifurcationAnomaly;
    bool ringAnomaly;
    bool measlesAnomaly; 

} ImageAnomalies;

void analyzeImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies);

void initializeAnomalyData(ImageAnomalies *anomalies);

bool scienceMode(ImageAuxData *aux);

void statsusage(const char * name);

#endif // _ANOMALY_STATS_H
