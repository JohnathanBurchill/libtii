#include "tiim.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fts.h>

// read image stats text files between date range, select data for science only imaging mode or all imagery, and average
void parserUsage(const char *program);


int main( int argc, char **argv)
{

    if (argc != 7)
    {
        parserUsage(argv[0]);
    }

    char satLetter = argv[1][0];
    if (satLetter != 'A' && satLetter != 'B' && satLetter != 'C')
    {
        parserUsage(argv[0]);
        goto cleanup;
    }
    char *beginDate = argv[2];
    char *endDate = argv[3];
    int scienceOnly = atoi(argv[4]);
    if (scienceOnly != 0 && scienceOnly != 1)
    {
        parserUsage(argv[0]);
        goto cleanup;
    }
    double averageIntervalMinutes = atof(argv[5]);
    if (averageIntervalMinutes <=0)
    {
        parserUsage(argv[0]);
        goto cleanup;
    }
    int paramToRead = atoi(argv[6]);
    if (paramToRead < 1 || paramToRead > 4)
    {
        parserUsage(argv[0]);
        goto cleanup;
    }

    char *path[2] = {NULL, NULL};
    path[0] = ".";
    FTS * fts = fts_open(path, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
    if (fts == NULL)
    {
        printf("Could not open folder.\n");
        goto cleanup;
    }
    FTSENT * f = fts_read(fts);
    int nameLength;
    
    double *times = NULL;
    double *values = NULL;
    size_t nValues = 0;
    double t, mh, mv, pah, pav, vph, vpv, vmh, vmv, vbh, vbv, vf;
    FILE *file = NULL;
    int valuesRead = 0;
    double epoch1970 = 2208988800.0;

    while (f != NULL)
    {
        if ( f->fts_namelen == 31 && f->fts_name[6] == satLetter && strncmp(f->fts_name, "SW_EFI", 6) == 0 && strncmp(f->fts_name + 15, "_image_stats.txt", 16) == 0 && (strncmp(f->fts_name + 7, beginDate, 8) >= 0 && strncmp(f->fts_name + 7, endDate, 8) <= 0))
        {
            // Very likely got an image_stats.txt file for the correct satellite and date range
            // Ignore errors: we read as many files as we can
            file = fopen(fts->fts_path, "r");
            if (file == NULL)
            {
                printf("Could not read file.\n");
                fts_close(fts);
                goto cleanup;
            }
            while((valuesRead = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &t, &mh, &mv, &pah, &pav, &vph, &vpv, &vmh, &vmv, &vbh, &vbv, &vf)) != EOF)
            {
                if (valuesRead != 12)
                    break;
                nValues++;
                times = (double*) realloc(times, nValues * sizeof(double));
                values = (double*) realloc(values, nValues * sizeof(double));
                if (times == NULL || values == NULL)
                {
                    printf("Could not allocate memory.");
                    fts_close(fts);
                    goto cleanup;
                }
                if (scienceOnly == 0 || (vph >= 4700.0 && vph <= 5300.0 && vpv >= 4700.0 && vpv <= 5300.0 && vmh <= -1000.0 && vmv <= -1000.0 && vbh <= -50.0 && vbv <= -50.0))
                times[nValues-1] = t + epoch1970;
                switch(paramToRead)
                {
                    case 1:
                        values[nValues-1] = mh;
                        break;
                    case 2:
                        values[nValues-1] = mv;
                        break;
                    case 3:
                        values[nValues-1] = pah;
                        break;
                    case 4:
                        values[nValues-1] = pav;
                        break;
                    default:
                        break;
                }

            }
            fclose(file);
            file = NULL;
        }
        f = fts_read(fts);
    }
    fts_close(fts);

    // Average the data
    size_t avgInd = 0;
    size_t intervalSamples = 0;
    double firstT;
    double deltaT = 60.0 * averageIntervalMinutes;
    double *avgTimes = NULL;
    double *avgValues = NULL;
    if (nValues > 0)
    {
        avgTimes = (double*)malloc(nValues * sizeof(double));
        avgValues = (double*)malloc(nValues * sizeof(double));
        if (avgTimes == NULL || avgValues == NULL)
        {
            printf("Cannot remember the averages.\n");
            goto cleanup;
        }
        memset(avgTimes, 0, nValues * sizeof(double));
        memset(avgValues, 0, nValues * sizeof(double));
        firstT = times[0];
        for (size_t i = 0; i < nValues; i++)
        {
            if (times[i] >= firstT + deltaT)
            {
                if (intervalSamples > 0)
                {
                    avgTimes[avgInd] /= (double)intervalSamples;
                    avgValues[avgInd] /= (double)intervalSamples;
                    avgInd++;
                }
                intervalSamples = 0;
                firstT += deltaT;
            }
            else
            {
                avgTimes[avgInd] += times[i];
                avgValues[avgInd] += values[i];
                intervalSamples++;
            }
        }
    }


    for (size_t i = 0; i < avgInd; i++)
    {
        printf("%lf %lf\n", avgTimes[i], avgValues[i]);
    }

    cleanup:
    if (file != NULL)
    {
        fclose(file);
    }
    if (times != NULL)
        free(times);
    if (values != NULL)
        free(values);
    if (avgTimes != NULL)
        free(avgTimes);
    if (avgValues != NULL)
        free(avgValues);

    return 0;
}

void parserUsage(const char *program)
{
    printf("\nTII Daily Image Statistics Parser %s compiled %s %s UTC\n", TIIM_VERSION, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("Usage: %s satelliteLetter startDate endDate scienceOnly averagingIntervalMinutes paramToRead\n", program);
    printf("Dates take the form yyyymmdd. scienceOnly is 0 or 1 (true or false) to toggle keeping measurements from TII science operations only (4700 V <= VPhos <= 5300 V, VMCP < -1000 V, VBias < -50 V.");
    printf("paramToRead:\n");
    printf(" 1: measles count H\n");
    printf(" 2: measles count V\n");
    printf(" 3: PA count H\n");
    printf(" 4: PA count V\n");
}
