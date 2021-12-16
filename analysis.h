#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct imageStats
{
    double maxValue;

    // anomalies
    int paCount;
    int cumulativePaCount;
    int measlesCount;
    int cumulativeMeaslesCount;

} ImageStats;

void analyzeImage(uint16_t *pixels, bool gotImage, double requestedMaxValue, ImageStats *stats);

#endif // _ANALYSIS_H
