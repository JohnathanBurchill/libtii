#ifndef _UTILITY_H
#define _UTILITY_H

#include "isp.h"

#include <time.h>

enum UTIL_CODES
{
    UTIL_OK = 0,
    UTIL_MOVIE_NAME_NO_DATE = -1,
    UTIL_MOVIE_NAME_NO_SATELLITE = -2,
    UTIL_DATE_PARSE = -3
};

int constructMovieFilename(char satellite, time_t dayStart, time_t dayEnd, const char *outputDir, char *movieFilename);

int dateToSecondsSince1970(const char * yyyymmdd, double *seconds);

bool ignoreTime(double time, double dayStart, double dayEnd);

bool scienceMode(ImageAuxData *aux);

#endif // _UTILITY_H