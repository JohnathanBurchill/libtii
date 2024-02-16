/*

    TIIM processing tools: tools/lp_sweeps/lp_sweeps.c

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

#include "lp_sweeps.h"

#include "tii.h"

#include "isp.h"
#include "import.h"
#include "utility.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

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

    SciencePackets sciencePackets;
    importScience(satDate, &sciencePackets);

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
    sprintf(dailyMeasurementsFilename, "%s/SW_EFI%s_lp_sweeps.txt", outputDir, satDate);
    dailyLpStatsFile = fopen(dailyMeasurementsFilename, "w");
    if (dailyLpStatsFile == NULL)
    {
        fprintf(stderr, "Could not open LP sweep output data file.\n");
        goto cleanup;
    }

    // time in sec since 1970, ion density probe 1, ion density probe 2, faceplate current
    fprintf(dailyLpStatsFile, "secondsSince1970 nSteps stepHeight StartBias signChangeStep samplesPerStep i1[...] i2[...]\n");

    LpSweepPacket *p = NULL;
    LpSweep sweep = {0};
    double t = 0.0;
    for (size_t i = 0; i <  sciencePackets.numberOfLpSweepPackets; i++)
    {
        p = (LpSweepPacket*)(sciencePackets.lpSweepPackets + i * LP_SWEEP_PACKET_SIZE);
        getLpSweepData(p, &sweep);
        t = sweep.dateTime.secondsSince1970;
        if (t < dayStart || t >= dayEnd)
            continue;
                
        fprintf(dailyLpStatsFile, "%lf", t);
        // Biases
        fprintf(dailyLpStatsFile, " %u %d %u %u %u", sweep.auxData.durationInSweepSteps, 
                sweep.auxData.heightOfBiasStep, sweep.auxData.startBiasInTmUnits, 
                sweep.auxData.stepAtWhichWeChangeSignOfStepHeight, 
                sweep.auxData.numberOfSweepSamplesBetweenSteps);
        // Currents probe 1
        for (int k = 0; k < 252; k++)
            fprintf(dailyLpStatsFile, " %d", sweep.currentSensor1[k]);
        // Currents probe 2
        for (int k = 0; k < 252; k++)
            fprintf(dailyLpStatsFile, " %d", sweep.currentSensor2[k]);
        fprintf(dailyLpStatsFile, "\n");
    }


    fflush(dailyLpStatsFile);

cleanup:
    fclose(dailyLpStatsFile);

    fflush(stdout);

    exit(0);
}

void usage(const char * name)
{
    printf("\nLP Sweep data%s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2024 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s <Xyyyymmdd> <outputDir>\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");

    return;
}

