/*

    TIIM processing tools: tools/lp_stats/lp_stats.c

    Copyright (C) 2024  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "lp_stats.h"

#include "tii.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        usage(argv[0]);
        exit(0);
    }

    int status = 0;

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);
    FILE *dailyLpStatsFile =  NULL;
    if (sourceLen != 9)
    {
        usage(argv[0]);
        exit(1);
    }

    char satellite = satDate[0];
    if (satellite != 'A' && satellite != 'B' && satellite != 'C')
    {
        printf("Invalid satellite letter %c. Expected A, B, or C\n", satellite);
        exit(1);
    }

    double max = -1.0;
    char *outputDir = argv[2];

    SciencePackets sciencePackets;
    LpTiiTimeSeries timeSeries;
    initLpTiiTimeSeries(&timeSeries);
    importScience(satDate, &sciencePackets);
    getLpTiiTimeSeries(satDate[0], &sciencePackets, &timeSeries);

    double dayStart = 0.0;
    double dayEnd = 0.0;
    // Filter images based on time of day if yyyymmdd was passed
    // Seconds since 1970
    if (dateToSecondsSince1970(satDate + 1, &dayStart))
    {
        fprintf(stderr, "Could not parse %s to a date.\n", satDate);
        goto cleanup;
    }
    dayEnd = dayStart + 86400.0; // ignore leap second on this day
 
    char dailyMeasurementsFilename[FILENAME_MAX];
    sprintf(dailyMeasurementsFilename, "%s/SW_EFI%s_lp_stats.txt", outputDir, satDate);
    dailyLpStatsFile = fopen(dailyMeasurementsFilename, "w");
    if (dailyLpStatsFile == NULL)
    {
        fprintf(stderr, "Could not open LP stats file.\n");
        goto cleanup;
    }

    // time in sec since 1970, ion density probe 1, ion density probe 2, faceplate current
    fprintf(dailyLpStatsFile, "secondsSince1970 ni1 ni2 ifp\n");
    double ifp = 0.0;

    double t2Hz = timeSeries.lpTiiTime2Hz[0];
    for (size_t i = 0; i < timeSeries.n2Hz; i++)
    {
        t2Hz = timeSeries.lpTiiTime2Hz[i];
        ifp = 0.0;
        for (int p = 0; p < 8; p++)
        {
            ifp += timeSeries.faceplateCurrent[8*i + p];
        }
        ifp /= 8.0;
        fprintf(dailyLpStatsFile, "%.2lf %.1lf %.1lf %lg\n", timeSeries.lpTiiTime2Hz[i], timeSeries.ionDensity1[i], timeSeries.ionDensity2[i], ifp);
    }
    fflush(dailyLpStatsFile);

cleanup:
    fclose(dailyLpStatsFile);
    freeLpTiiTimeSeries(&timeSeries);

    fflush(stdout);

    exit(0);
}

void usage(const char * name)
{
    printf("\nLP Daily currents%s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2024 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");

    return;
}

