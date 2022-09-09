/*

    TIIM processing library: lib/tii/utility.c

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

#include "utility.h"

#include "tii.h"
#include "isp.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int constructMovieFilename(char satellite, time_t dayStart, time_t dayEnd, const char *outputDir, char *movieFilename)
{

    char startDate[255];
    char stopDate[255];
    memset(startDate, 0, 16);
    memset(stopDate, 0, 16);

    struct tm *date = NULL;

    // This is like the unix time functions -> the returned pointer is valid until the next call.
    date = gmtime(&dayStart);
    sprintf(startDate, "%04d%02d%02dT%02d%02d%02d", date->tm_year+1900, date->tm_mon+1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);

    date = gmtime(&dayEnd);
    sprintf(stopDate, "%04d%02d%02dT%02d%02d%02d", date->tm_year+1900, date->tm_mon+1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);

    sprintf(movieFilename, "%s/SW_OPER_EFI%cTIIMOV_%s_%s_%s.mp4", outputDir, satellite, startDate, stopDate, TII_LIB_VERSION_STRING);

    return UTIL_OK;

}

int dateToSecondsSince1970(const char * yyyymmdd, double *seconds)
{
    int year, month, day;
    sscanf(yyyymmdd, "%4d%2d%2d", &year, &month, &day);
    if (year < 2010 || month < 1 || month > 12 || day < 1 || day > 31)
        return UTIL_DATE_PARSE;

    struct tm timeStruct = {0};
    timeStruct.tm_year = year - 1900;
    timeStruct.tm_mon = month - 1;
    timeStruct.tm_mday = day;


    *seconds = (double) timegm(&timeStruct);
    
    return UTIL_OK;

}

bool ignoreTime(double time, double dayStart, double dayEnd)
{
        // discard packets that are not of this day
        return (time < dayStart || time > dayEnd);
}

bool scienceMode(ImageAuxData *aux)
{
    // Unable to check AGC from image ISPs. Should be on for science mode, but will allow non-AGC ops.
    // MCP voltage less than -1000 V
    // Phosphor voltage > 3900 V
    // Bias voltage < -50 V
    return aux->McpVoltageMonitor < -1000.0 && aux->PhosphorVoltageMonitor > 3900 && aux->BiasGridVoltageMonitor < -50.0;

}


void binaryView(const char *name, long nBytes, uint8_t *array, int rowSize, int startRow, int endRow, int startCol, int endCol)
{
    printf("Array: %s\n", name);
    printf("N\t");
    for (int j = startCol; j < endCol; j++)
        printf("%3d      ", j);
    printf("\n");

    uint8_t *p = array;
    long index = 0;
    for (int i = startRow; i < endRow; i++)
    {
        printf("%d\t", i);
        for (int j = startCol; j < endCol; j++)
        {
            index = rowSize*i + j;
            if (index < nBytes)
            {
                for (int b = 7; b >= 0; b--)
                    printf("%1d", (*(array+index) >> b) & 0x01);
                printf(" ");
            }
            else
                printf("--------");
        }
        printf("\n");
    }

}

