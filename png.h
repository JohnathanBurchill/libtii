#ifndef _PNG_H
#define _PNG_H

#include <stdint.h>
#include <spng.h>

struct spng_plte getColorTable();

int writePng(const char * filename, uint8_t *pixels, int width, int height, struct spng_plte * colorTable);


#endif // _PNG_H
