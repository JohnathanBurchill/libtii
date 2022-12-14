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
#include <time.h>

#define VERSION_STRING "1.0.0"

// read image stats text files between date range, select data for science only imaging mode or all imagery, and average
void parserUsage(const char *program);

int sortEm(const FTSENT **first, const FTSENT **second);
int sortEmReverse(const FTSENT **first, const FTSENT **second);
void getYearAndWeek(const char *filename, int *year, int *week);

int main( int argc, char **argv)
{
    FILE *file = NULL;

    bool reverseWeekOrder = false;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0 || strcmp(argv[i], "--help") == 0)
        {
            parserUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }
    if (argc != 3 && argc != 4)
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
    time_t sec = time(NULL);
    struct tm *now = gmtime(&sec);
    int y = atoi(year);
    int thisYear = now->tm_year + 1900;
    if (y < 2015 || y > thisYear)
    {
        fprintf(stderr, "%s: Requested year must be from 2015 to %d\n", argv[0], thisYear);
        goto cleanup;
    }

    if (argc == 4)
    {
        if (strcmp(argv[3], "--reverse") == 0)
        {
            reverseWeekOrder = true;            
        }
        else
        {
            fprintf(stderr, "%s: unknown option.\n", argv[0]);
            goto cleanup;
        }
    }

    char *path[2] = {NULL, NULL};
    path[0] = ".";
    FTS * fts = NULL;
    if (reverseWeekOrder == true)
        fts = fts_open(path, FTS_LOGICAL | FTS_NOCHDIR | FTS_NOSTAT, &sortEmReverse);
    else
        fts = fts_open(path, FTS_LOGICAL | FTS_NOCHDIR | FTS_NOSTAT, &sortEm);

    if (fts == NULL)
    {
        printf("Could not open folder.\n");
        exit(EXIT_SUCCESS);
    }

    FTSENT * f = fts_read(fts);
    
    // From man getline
    char *line = NULL;
    ssize_t linelen = 0;
    size_t linecap = 0;

    // Loop through files
    bool gotStart = false;
    bool gotTimeRange = false;
    bool newWeek = true;
    bool newYear = true;
    char thetime[255] = {0};
    char thedate[255] = {0};
    int fileYear = 0;
    int fileWeek = 0;
    ssize_t valuesRead = 0;

    while (f != NULL)
    {
        if (strncmp(f->fts_name, "EMAIL_ON_OFF_times_Swarm-", 25) == 0 && f->fts_name[25] == satLetter && strcmp(f->fts_name + f->fts_namelen - 4, ".txt") == 0 && ((strncmp(f->fts_name + f->fts_namelen - 14, year, 4) == 0 && *(f->fts_name + f->fts_namelen - 10) == '_') || strncmp(f->fts_name + f->fts_namelen - 13, year, 4) == 0))
        {
            file = fopen(fts->fts_path, "r");
            if (file == NULL)
            {
                fprintf(stderr, "Could not read file.\n");
                f = fts_read(fts);
                continue;
            }
            gotStart = false;            
            gotTimeRange = true;

            getYearAndWeek(f->fts_name, &fileYear, &fileWeek);
            while((valuesRead = getline(&line, &linecap, file)) > 0)
            {
                if (valuesRead != 50)
                    continue;

                if (newYear == true)
                {
                    fprintf(stdout, "<ul>\n");
                    newYear = false;
                }
                if (newWeek == true)
                {
                    snprintf(thedate, 11, "%s", line + 29);
                    thedate[4] = '-';
                    thedate[7] = '-';
                    fprintf(stdout, "<li>Week %d (starting %s)\n  <ul>\n", fileWeek, thedate);
                    newWeek = false;
                }
                if (strncmp(line + 21, "ON", 2)  == 0)
                {
                    snprintf(thetime, 20, "%s", line + 29);
                    thetime[4] = '-';
                    thetime[7] = '-';
                    fprintf(stdout, "    <li>%s UT", thetime);
                    gotStart = true;
                }
                else if (strncmp(line + 21, "OF", 2) == 0)
                {
                    snprintf(thetime, 20, "%s", line + 29);
                    thetime[4] = '-';
                    thetime[7] = '-';
                    if (gotStart == false)
                    {
                        // Operations continued from previous week
                        fprintf(stdout, "    <li>...");
                    }
                    fprintf(stdout, " - %s UT</li>\n", thetime);
                    gotStart = false;
                }
            }
            fclose(file);
            file = NULL;
            if (gotStart == true)
            {
                // Operations continued to the next week
                fprintf(stdout, " - ...</li>\n");
            }
            if (newWeek == false)
            {
                fprintf(stdout, "  </ul>\n</li>\n");
            }
            newWeek = true;
        }
        f = fts_read(fts);
    }
    fts_close(fts);
    if (newYear == false)
    {
        fprintf(stdout, "</ul>\n");
        newYear = false;
    }

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
    printf("Usage: %s satelliteLetter year [--reverse]\n", program);
    printf("Options:\n");
    printf("\t--about prints this message\n");
    printf("\t--reverse prints most recent weeks first\n");

    return;
}

int sortEm(const FTSENT **first, const FTSENT **second)
{
    if (first == NULL || second == NULL)
        return 0;

    const FTSENT *a = *first;
    const FTSENT *b = *second;

    if (a == NULL || b == NULL)
        return 0;
    // order does not matter if either is not an operations file
    if (strncmp(a->fts_name, "EMAIL_ON_OFF_times", 18) != 0 && strncmp(b->fts_name, "EMAIL_ON_OFF_times", 18) != 0)
        return 0;
    else if (strncmp(a->fts_name, "EMAIL_ON_OFF_times", 18) != 0)
        return -1;
    else if (strncmp(b->fts_name, "EMAIL_ON_OFF_times", 18) != 0)
        return 1;

    // Get year and week
    int weekA = 0;
    int yearA = 0;
    int weekB = 0;
    int yearB = 0;
    getYearAndWeek(a->fts_name, &yearA, &weekA);
    getYearAndWeek(b->fts_name, &yearB, &weekB);
    // printf("%s %s\n", a->fts_name, b->fts_name);

    int comp = 0;
    if (yearA > yearB)
        comp = 1;
    else if (yearA < yearB)
        comp = -1;
    else
    {
        if (weekA > weekB)
            comp = 1;
        else if (weekA < weekB)
            comp = -1;
        else
            comp = 0;
    }

    // Oldest to newest
    return comp;
}

int sortEmReverse(const FTSENT **first, const FTSENT **second)
{
    return -sortEm(first, second);
}

void getYearAndWeek(const char *filename, int *year, int *week)
{
    if (strlen(filename) < 39)
        return;

    int weekDigits = 1;
    char weekStr[16] = {0};
    char yearStr[16] = {0};

    if (*(filename + 33) != '_')
        weekDigits = 2;
    
    snprintf(weekStr, weekDigits + 1, "%s", filename + 32);

    snprintf(yearStr, 5, "%s", filename + 33 + weekDigits);

    if (week != NULL)
        *week = atoi(weekStr); 
    if (year != NULL)
        *year = atoi(yearStr);  

    return;
}