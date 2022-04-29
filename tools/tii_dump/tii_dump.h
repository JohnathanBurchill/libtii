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
