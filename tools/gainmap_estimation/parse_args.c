/*

    TIIM processing tools: tools/gainmap_estimation/parse_args.c

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
#include "parse_args.h"

#include "utility.h"
#include "gainmap.h"
#include "usage.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


void parseArgs(State *state, int argc, char **argv)
{
    state->calDataH.max = -1.0;
    state->calDataV.max = -1.0;
    state->reciprocal = true;
    state->sensorToPrint = 'H';
    state->calDataH.minBias = -19.0;
    state->calDataV.minBias = -19.0;
    state->calDataH.maxBias = 0.0;
    state->calDataV.maxBias = 0.0;
    state->calDataH.deltaBias = -0.5;
    state->calDataV.deltaBias = -0.5;

    double val = 0;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp("--help", argv[i]) == 0)
        {
            usage(argv[0]);
            exit(0);
        }
        else if (strncmp("--sensor=", argv[i], 9) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 10)
            {
                fprintf(stderr, "Unable to parse %s\n", argv[i]);
                exit(1);
            }
            state->sensorToPrint = argv[i][9];
        }
        else if (strcmp("--previous-gainmap-voltage-steps", argv[i]) == 0)
        {
            state->nOptions++;
            state->printPreviousStepIndices = true;
        }
        else if (strncmp("--csv=", argv[i], 6) == 0)
        {
            state->nOptions++;
            state->exportCsv = true;
            if (strlen(argv[i]) < 9)
            {
                usage(argv[0]);
                exit(1);
            }
            if (strcmp("last", argv[i] + 6) == 0)
                state->csvUsePreviousStepIndices = true;
            else
            {
                // =indH,indV
                // From strsep man page
                char **ap;
                char *tokens[2] = {NULL, NULL};
                char *inputstring;
                inputstring = argv[i] + 6;
                for (ap = tokens; (*ap = strsep(&inputstring, ",")) != NULL;)
                    if (**ap != '\0')
                        if (++ap >= &tokens[2])
                            break;
                if (tokens[1] == NULL)
                {
                    printf("Unable to parse %s\n", argv[i]);
                }

                state->csvIndH = atoi(tokens[0]);
                state->csvIndV = atoi(tokens[1]);
                if (state->csvIndH < 0 || state->csvIndH >= GAINMAP_MAX_VOLTAGES || state->csvIndV < 0 || state->csvIndV >= GAINMAP_MAX_VOLTAGES)
                {
                    printf("Gainmap voltage step index must be between 0 and %d\n", GAINMAP_MAX_VOLTAGES - 1);
                    exit(1);
                }
            }
        }
        else if (strcmp("--gainmaps", argv[i]) == 0)
        {
            state->nOptions++;
            state->printGainMapTable = true;
        }
        else if (strcmp("--print-image-info", argv[i]) == 0)
        {
            state->nOptions++;
            state->printImageInfo = true;
        }
        else if (strncmp("--minimum-image-maximum-H=", argv[i], 26) == 0 || strncmp("--minimum-image-maximum-V=", argv[i], 26) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 27)
            {
                usage(argv[0]);
                exit(1);
            }
            val = atoi(argv[i]+26);
            if (argv[i][24] == 'H')
                state->calDataH.minImageMaximum = val;
            else
                state->calDataV.minImageMaximum = val;
        }
        else if (strncmp("--print-image=", argv[i], 14) == 0)
        {
            state->nOptions++;
            state->printImage = true;
            if (strlen(argv[i]) < 15)
            {
                usage(argv[0]);
                exit(1);
            }
            state->imageIndexToPrint = atoi(argv[i]+14);
            if (state->imageIndexToPrint < 0 || state->imageIndexToPrint >= GAINMAP_MAX_VOLTAGES)
            {
                printf("Image index to print must be between 0 and %d\n", GAINMAP_MAX_VOLTAGES - 1);
                exit(1);
            }
        }
        else if (strcmp("--raw-image", argv[i]) == 0)
        {
            state->nOptions++;
            state->reciprocal = false;
        }
        else if (strncmp("--png=", argv[i], 6) == 0)
        {
            state->nOptions++;
            state->writePng = true;
            if (strlen(argv[i]) < 9)
            {
                usage(argv[0]);
                exit(1);
            }
            if (strcmp("all", argv[i] + 6) == 0)
                state->pngAllSteps = true;
            else if (strcmp("last", argv[i] + 6) == 0)
                state->usePreviousStepIndices = true;
            else
            {
                // =indH,indV
                // From strsep man page
                char **ap;
                char *tokens[2] = {NULL, NULL};
                char *inputstring;
                inputstring = argv[i] + 6;
                for (ap = tokens; (*ap = strsep(&inputstring, ",")) != NULL;)
                    if (**ap != '\0')
                        if (++ap >= &tokens[2])
                            break;
                if (tokens[1] == NULL)
                {
                    printf("Unable to parse %s\n", argv[i]);
                }

                state->pngIndH = atoi(tokens[0]);
                state->pngIndV = atoi(tokens[1]);
                if (state->pngIndH < 0 || state->pngIndH >= GAINMAP_MAX_VOLTAGES || state->pngIndV < 0 || state->pngIndV >= GAINMAP_MAX_VOLTAGES)
                {
                    printf("Gainmap voltage step index must be between 0 and %d\n", GAINMAP_MAX_VOLTAGES - 1);
                    exit(1);
                }
            }
        }
        else if (strcmp("--crop", argv[i]) == 0)
        {
            state->nOptions++;
            state->crop = true;
        }
        else if (strncmp("--maxH=", argv[i], 7) == 0 || strncmp("--maxV=", argv[i], 7) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 8)
            {
                usage(argv[0]);
                exit(1);
            }
            val = atof(argv[i]+7);
            if (argv[i][5] == 'H')
                state->calDataH.max = val;
            else
                state->calDataV.max = val;
        }
        else if (strncmp("--crop-dx-H=", argv[i], 12) == 0 || strncmp("--crop-dx-V=", argv[i], 12) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 13)
            {
                usage(argv[0]);
                exit(1);
            }
            val = atof(argv[i]+12);
            if (argv[i][10] == 'H')
                state->calDataH.cropMask.dx0 = val;
            else
                state->calDataV.cropMask.dx0 = val;
        }
        else if (strncmp("--crop-dy-H=", argv[i], 12) == 0 || strncmp("--crop-dy-V=", argv[i], 12) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 13)
            {
                usage(argv[0]);
                exit(1);
            }
            val = atof(argv[i]+12);
            if (argv[i][10] == 'H')
                state->calDataH.cropMask.dy0 = val;
            else
                state->calDataV.cropMask.dy0 = val;
        }
        else if (strncmp("--crop-drx-H=", argv[i], 13) == 0 || strncmp("--crop-drx-V=", argv[i], 13) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 14)
            {
                usage(argv[0]);
                exit(1);
            }
            val = atof(argv[i]+13);
            if (argv[i][11] == 'H')
                state->calDataH.cropMask.drx = val;
            else
                state->calDataV.cropMask.drx = val;
        }
        else if (strncmp("--crop-dry-H=", argv[i], 13) == 0 || strncmp("--crop-dry-V=", argv[i], 13) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 14)
            {
                usage(argv[0]);
                exit(1);
            }
            val = atof(argv[i]+13);
            if (argv[i][11] == 'H')
                state->calDataH.cropMask.dry = val;
            else
                state->calDataV.cropMask.dry = val;
        }
        else if (strncmp("--min-bias=", argv[i], 11) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 12)
            {
                usage(argv[0]);
                exit(1);
            }
            state->calDataH.minBias = atof(argv[i]+11);
            state->calDataV.minBias = state->calDataH.minBias;
        }
        else if (strncmp("--max-bias=", argv[i], 11) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 12)
            {
                usage(argv[0]);
                exit(1);
            }
            state->calDataH.maxBias = atof(argv[i]+11);
            state->calDataV.maxBias = state->calDataH.maxBias;
        }
        else if (strncmp("--delta-bias=", argv[i], 13) == 0)
        {
            state->nOptions++;
            if (strlen(argv[i]) < 14)
            {
                usage(argv[0]);
                exit(1);
            }
            state->calDataH.deltaBias = atof(argv[i]+13);
            state->calDataV.deltaBias = state->calDataH.deltaBias;
        }
        else if (strncmp("--", argv[i], 2) == 0)
        {
            printf("Unrecognized option %s\n", argv[i]);
            exit(1);
        }
    }

    if (argc - state->nOptions != 6)
    {
        usage(argv[0]);
        exit(0);
    }

    int status = 0;

    state->satellite = argv[1][0];
    if (state->satellite != 'A' && state->satellite != 'B' && state->satellite != 'C')
    {
        printf("Invalid satellite letter %c. Expected A, B, or C\n", state->satellite);
        exit(1);
    }

    state->date1 = argv[2];
    state->date2 = argv[3];

    if (dateToSecondsSince1970(state->date1, &state->t1))
    {
        fprintf(stderr, "Could not parse %s to a date.\n", state->date1);
        exit(1);
    }
    if (dateToSecondsSince1970(state->date2, &state->t2))
    {
        fprintf(stderr, "Could not parse %s to a date.\n", state->date2);
        exit(1);
    }

    state->outDir = argv[4];

    state->cropGeometryFilename = argv[5];
    if (access(state->cropGeometryFilename, R_OK) != 0)
    {
        fprintf(stderr, "Unable to read %s\n", state->cropGeometryFilename);
        exit(1);
    }

    state->nDates = 1 + (int)((state->t2 - state->t1)/ 86400.0);

    fprintf(stderr, "Processing imagery spanning %d %s\n", state->nDates, state->nDates == 1 ? "day" : "days"); 

    // Check valid gain map ID
    int nGainMaps = 0;
    double *gainMapTimeArray = NULL;
    status = gainMapTimes(state->satellite, &nGainMaps, &gainMapTimeArray);
    if (status != 0)
    {
        printf("Unable to load gain map times for Swarm %d\n", state->satellite);
        exit(0);
    }

    state->calDataH.nBiases = (state->calDataH.maxBias - state->calDataH.minBias) / fabs(state->calDataH.deltaBias);
    state->calDataV.nBiases = (state->calDataV.maxBias - state->calDataV.minBias) / fabs(state->calDataV.deltaBias);


    return;
}
