#ifndef _FILTERS_H
#define _FILTERS_H

#include <stdarg.h>
#include <stdbool.h>


// Image Filters

double identityFilter(int k, void *pixelBuf, bool *missing, void *args);

double paAngularSpectrumFilter(int k, void *pixelBuf, bool *missing, void *args);


#endif // _FILTERS_H
