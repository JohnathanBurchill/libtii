#ifndef _PNG_H
#define _PNG_H

#include "draw.h"
#include "spng.h"

#include <stdint.h>

struct spng_plte getColorTable();

int writePng(const char * filename, Image *image, struct spng_plte * colorTable);


#endif // _PNG_H
