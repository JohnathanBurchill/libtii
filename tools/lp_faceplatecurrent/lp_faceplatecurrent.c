/*

    TIIM processing tools: tools/lp_faceplatecurrent/lp_faceplatecurrent.c

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

#include "lp_faceplatecurrent.h"

#include "tii.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "timeseries.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <cdf.h>

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

    char *outputDir = argv[2];
    SciencePackets sciencePackets = {0};
    LpTiiTimeSeries timeSeries = {0};
    initLpTiiTimeSeries(&timeSeries);
 

    char dailyMeasurementsFilename[FILENAME_MAX];
    sprintf(dailyMeasurementsFilename, "%s/SW_EXTD_EFI%c_LP_FP_%sT000000_%sT235959_0000.cdf", outputDir, satellite, satDate+1, satDate+1);
    if (access(dailyMeasurementsFilename, F_OK) == 0)
    {
        fprintf(stderr, "CDF file exists. Exiting.\n");
        goto cleanup;
    }

    importScience(satDate, &sciencePackets);
    status = getLpTiiTimeSeries(satellite, &sciencePackets, &timeSeries);
    if (status != IMPORT_OK)
    {
        fprintf(stderr, "Unable to load LP/TII time series data.\n");
        goto cleanup;
    }

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

    double t = 0.0;
    ssize_t startInd = -1;
    ssize_t stopInd = -1;
    for (size_t i = 0; i <  timeSeries.n16Hz; i++)
    {
        t = timeSeries.lpTiiTime16Hz[i];
        if (t < dayStart || t >= dayEnd)
        {
            continue;
        }
        if (startInd <0)
            startInd = (ssize_t)i;
        stopInd = i;
//        fprintf(stderr, "%lf %lg\n", t, timeSeries.faceplateCurrent[i]);
    }
    fprintf(stderr, "start: %ld stop: %ld\n", startInd, stopInd);


cleanup:
    // CDF file cleanup
    // free memory
    freeLpTiiTimeSeries(&timeSeries);

    fflush(stdout);

    exit(0);
}

void usage(const char * name)
{
    printf("\nLP Faceplate Current data%s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2024 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s <Xyyyymmdd> <outputDir>\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");

    return;
}

