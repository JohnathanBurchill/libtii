/*

    TIIM processing tools: tools/tii_dump/tii_dump.h

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

#ifndef _TII_DUMP_H
#define _TII_DUMP_H

#include <stdlib.h>
#include <stdio.h>

enum packetType
{
    PARAM_NONE = 0,
    PARAM_2HZ,
    PARAM_16Hz,
    PARAM_CONFIG,
    PARAM_SWEEP,
    PARAM_OFFSET
};

enum numberType
{
    PARAM_NOTYPE = 0,
    PARAM_UINT8,
    PARAM_INT8,
    PARAM_UINT16,
    PARAM_INT16,
    PARAM_UINT32,
    PARAM_INT32,
    PARAM_UINT64,
    PARAM_INT64,
    PARAM_FLOAT,
    PARAM_DOUBLE
};

typedef struct tiiParameter
{
    int id;
    int packetType;
    int numberType;
    char *name;
    char *description;
    size_t offset;
} TiiParameter;

void parameterList(size_t nParameters, TiiParameter *parameters);

void usage(const char * name);

void writeWithSpaceAsUnderscore(FILE *fd, char *s);

#endif // _TII_DUMP_H
