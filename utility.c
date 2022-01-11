#include "utility.h"

#include "isp.h"
#include "tiim.h"

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

    sprintf(movieFilename, "%s/SW_OPER_EFI%cTIIMOV_%s_%s_%s.mp4", outputDir, satellite, startDate, stopDate, TIIM_VERSION);

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

void usage(const char * name)
{
    printf("\nTII Movie Generator Version %s compiled %s %s UTC\n", TIIM_VERSION, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s SW_OPER_EFIXDDD_0__yyyyMMddThhmmss_yyyyMMddThhmmss_vvvv.HDR maxSignal outputDir [-f] \n", name);
    printf("\nor\n");
    printf("\n  %s Xyyyymmdd maxSignal outputDir [-f]\n", name);
    printf("\n");
    printf("In the first form DDD is either \"NOM\" or \"TIC\"\n");
    printf("In the second form X designates the Swarm satellite (A, B or C).\n");
    printf("Set maxSignal to -1 for autoscaling the TII imagery.\n");
    printf("\"-f\" forces overwriting an extant TII movie file.\n");

    return;
}
