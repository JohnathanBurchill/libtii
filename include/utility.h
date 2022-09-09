/*

    TIIM processing library: include/utility.h

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

void binaryView(const char *name, long nBytes, uint8_t *array, int rowSize, int startRow, int endRow, int startCol, int endCol);

#endif // _UTILITY_H
