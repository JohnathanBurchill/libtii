/*

    TIIM processing tools: tools/get_operations/get_operations.c

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

// Adapted from tools/get_image_stats/get_image_stats.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <fts.h>
#include <stdbool.h>

#define VERSION_STRING "1.0.0"

// read image stats text files between date range, select data for science only imaging mode or all imagery, and average
void parserUsage(const char *program);

int sortEm(FTSENT **first, FTSENT **second);

int main( int argc, char **argv)
{
    FILE *file = NULL;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0 || strcmp(argv[i], "--help") == 0)
        {
            parserUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    if (argc != 3)
    {
        parserUsage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    char satLetter = argv[1][0];
    if (satLetter != 'A' && satLetter != 'B' && satLetter != 'C')
    {
        parserUsage(argv[0]);
        goto cleanup;
    }
    char *year = argv[2];

    char *path[2] = {NULL, NULL};
    path[0] = ".";
    FTS * fts = fts_open(path, FTS_LOGICAL | FTS_NOCHDIR | FTS_NOSTAT, NULL);
    if (fts == NULL)
    {
        printf("Could not open folder.\n");
        exit(EXIT_SUCCESS);
    }

    FTSENT * f = fts_read(fts);
    int nameLength;
    
    size_t nValues = 0;
    double t, mh, mv, pah, pav, vph, vpv, vmh, vmv, vbh, vbv, vf;
    int valuesRead = 0;
    double epoch1970 = 2208988800.0;

    // From man getline
    char *line = NULL;
    ssize_t linelen = 0;
    size_t linecap = 0;

    // Loop through files
    bool gotStart = false;
    bool gotTimeRange = false;
    bool newWeek = true;
    char thetime[255] = {0};
    char thedate[255] = {0};

    fprintf(stderr, "%s", year);

    while (f != NULL)
    {
        if (strncmp(f->fts_name, "EMAIL_ON_OFF_times_Swarm-", 25) == 0 && f->fts_name[25] == satLetter && strcmp(f->fts_name + f->fts_namelen - 4, ".txt") == 0 && ((strncmp(f->fts_name + f->fts_namelen - 14, year, 4) == 0 && *(f->fts_name + f->fts_namelen - 10) == '_') || strncmp(f->fts_name + f->fts_namelen - 13, year, 4) == 0))
        {
            // Ignore errors: we read as many files as we can
            file = fopen(fts->fts_path, "r");
            if (file == NULL)
            {
                fprintf(stderr, "Could not read file.\n");
                f = fts_read(fts);
                continue;
            }
            gotStart = false;            
            gotTimeRange = true;

            while((valuesRead = getline(&line, &linecap, file)) > 0)
            {
                if (valuesRead < 48)
                    continue;

                if (strncmp(line + 21, "ON", 2)  == 0)
                {
                    snprintf(thetime, 20, "%s", line + 29);
                    thetime[4] = '-';
                    thetime[7] = '-';
                    if (newWeek == true)
                    {
                        snprintf(thedate, 11, "%s", line + 29);
                        thedate[4] = '-';
                        thedate[7] = '-';
                        fprintf(stdout, "<li>Week of %s\n  <ul>\n    <li>", thedate);
                        newWeek = false;
                    }
                    else
                    {
                        fprintf(stdout, "    <li>");
                    }
                    fprintf(stdout, "%s UT", thetime);
                    gotStart = true;
                }
                else if (strncmp(line + 21, "OF", 2) == 0 && gotStart == true)
                {
                    snprintf(thetime, 20, "%s", line + 29);
                    thetime[4] = '-';
                    thetime[7] = '-';
                    fprintf(stdout, " - %s UT</li>\n", thetime);
                    gotStart = false;
                }
            }
            fclose(file);
            file = NULL;
            if (newWeek == false)
            {
                fprintf(stdout, "  </ul>\n</li>\n");
            }
            newWeek = true;
        }
        f = fts_read(fts);
    }
    fts_close(fts);

    cleanup:
    if (file != NULL)
    {
        fclose(file);
    }
    if (line != NULL)
        free(line);

    return 0;
}

void parserUsage(const char *program)
{
    printf("\nTII Daily Operations Parser %s compiled %s %s UTC\n", VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("Usage: %s satelliteLetter year\n", program);

    return;
}

int sortEm(FTSENT **first, FTSENT **second)
{
    if (first == NULL || second == NULL)
        return 0;

    FTSENT *a = *first;
    FTSENT *b = *second;
    if (a == NULL || b == NULL)
        return 0;

    return strcmp(a->fts_name, b->fts_name);
}