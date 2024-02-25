/*

    TIIM processing tools: tools/lp_faceplatecurrent/lp_faceplatecurrent.h

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

#ifndef _LP_FACEPLATECURRENT_H
#define _LP_FACEPLATECURRENT_H

#include <cdf.h>

#define NUM_EXPORT_VARIABLES 2
#define SOFTWARE_VERSION_STRING "0.1"

typedef struct varAttr {
    char * name;
    char * type;
    char * units;
    char * desc;
    double validMin;
    double validMax;
} varAttr;

void usage(const char * name);
CDFstatus createVarFrom1DVar(CDFid id, char *name, long dataType, long startIndex, long stopIndex, void *buffer);
void printErrorMessage(CDFstatus status);
CDFstatus addgEntry(CDFid id, long attrNum, long entryNum, const char *entry);

CDFstatus addVariableAttributes(CDFid id, varAttr attr);

void addAttributes(CDFid id, const char *calVersion, const char *satellite, const char *version, double minTime, double maxTime);

#endif // _LP_FACEPLATECURRENT_H
