/*

    TIIM processing tools: tools/process_anomaly_stats/process_anomaly_stats.c

    Copyright (C) 2022  Johnathan K Burchill

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fts.h>

#define TII_ANOMALY_PARSER_VERSION "1.0"

// read image stats text files between date range, select data for science only imaging mode or all imagery, and average
void parserUsage(const char *program);

int sortEm(const void *, const void *);

int main( int argc, char **argv)
{
    FILE *file = NULL;

    if (argc != 3)
    {
        parserUsage(argv[0]);
        goto cleanup;
    }

    char satLetter = argv[1][0];
    if (satLetter != 'A' && satLetter != 'B' && satLetter != 'C')
    {
        parserUsage(argv[0]);
        goto cleanup;
    }
    int monthsPerBin = atoi(argv[2]);
    if (monthsPerBin < 1)
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
    
    double nValues = 0.0;
    double t, pah, pav, mh, mv, cwh, cwv, bah, bav, uawh, uawv, lawh, lawv, uax1h, uax1v, uay1h, uay1v, uax2h, uax2v, uay2h, uay2v, lax1h, lax1v, lay1h, lay1v, lax2h, lax2v, lay2h, lay2v;
    int valuesRead = 0;

    double ta = 0, paha=0.0, pava=0.0, mha=0.0, mva=0.0, cwha=0.0, cwva=0.0, baha=0.0, bava=0.0, uawha=0.0, uawva=0.0, lawha=0.0, lawva=0.0, uax1ha=0.0, uax1va=0.0, uay1ha=0.0, uay1va=0.0, uax2ha=0.0, uax2va=0.0, uay2ha=0.0, uay2va=0.0, lax1ha=0.0, lax1va=0.0, lay1ha=0.0, lay1va=0.0, lax2ha=0.0, lax2va=0.0, lay2ha=0.0, lay2va=0.0;

    while (f != NULL)
    {
        if ( f->fts_namelen == 41 && f->fts_name[6] == satLetter && strncmp(f->fts_name, "SW_EFI", 6) == 0 && strncmp(f->fts_name + 15, "_imaging_anomaly_stats.txt", 16) == 0)
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
            fseek(file, 163, SEEK_SET);
            while((valuesRead = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &t, &pah, &pav, &mh, &mv, &cwh, &cwv, &bah, &bav, &uawh, &uawv, &lawh, &lawv, &uax1h, &uax1v, &uay1h, &uay1v, &uax2h, &uax2v, &uay2h, &uay2v, &lax1h, &lax1v, &lay1h, &lay1v, &lax2h, &lax2v, &lay2h, &lay2v)) != EOF)
            {
                if (valuesRead != 29)
                    break;                    
                nValues += 1.0;
                ta += t;
                paha += pah;
                pava += pav;
                mha += mh;
                mva += mv;
                cwha += cwh;
                cwva += cwv;
                baha += bah;
                bava += bav;
                uawha += uawh;
                uawva += uawv;
                lawha += lawh;
                lawva += lawv;
                uax1ha += uax1h;
                uax1va += uax1v;
                uay1ha += uay1h;
                uay1va += uay1v;
                uax2ha += uax2h;
                uax2va += uax2v;
                uay2ha += uay2h;
                uay2va += uay2v;
                lax1ha += lax1h;
                lax1va += lax1v;
                lay1ha += lay1h;
                lay1va += lay1v;
                lax2ha += lax2h;
                lax2va += lax2v;
                lay2ha += lay2h;
                lay2va += lay2v;

            }
            fclose(file);
            file = NULL;
        }
        f = fts_read(fts);
    }
    fts_close(fts);


    // Averages
    if (nValues > 0)
    {
        ta /= nValues;
        paha /= nValues;
        pava /= nValues;
        mha /= nValues;
        mva /= nValues;
        cwha /= nValues;
        cwva /= nValues;
        baha /= nValues;
        bava /= nValues;
        uawha /= nValues;
        uawva /= nValues;
        lawha /= nValues;
        lawva /= nValues;
        uax1ha /= nValues;
        uax1va /= nValues;
        uay1ha /= nValues;
        uay1va /= nValues;
        uax2ha /= nValues;
        uax2va /= nValues;
        uay2ha /= nValues;
        uay2va /= nValues;
        lax1ha /= nValues;
        lax1va /= nValues;
        lay1ha /= nValues;
        lay1va /= nValues;
        lax2ha /= nValues;
        lax2va /= nValues;
        lay2ha /= nValues;
        lay2va /= nValues;

        // print values
        printf("%ld %lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n", (long)nValues, ta, paha, pava, mha, mva, cwha, cwva, baha, bava, uawha, uawva, lawha, lawva, uax1ha, uax1va, uay1ha, uay1va, uax2ha, uax2va, uay2ha, uay2va, uax1ha, uax1va, uay1ha, uay1va, uax2ha, uax2va, uay2ha, uay2va);
    }


    cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return 0;
}

void parserUsage(const char *program)
{
    printf("\nTII Daily Image Anomaly Parser %s compiled %s %s UTC\n", TII_ANOMALY_PARSER_VERSION, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("Usage: %s satelliteLetter monthsPerBin\n", program);
}

int sortEm(const void *first, const void *second)
{
    double a = *((double*)first);
    double b = *((double*)second);
    if (a > b)
        return 1;
    else if (a == b)
        return 0;
    else
        return -1;
}