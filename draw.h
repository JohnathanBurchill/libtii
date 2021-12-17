#ifndef _DRAW_H
#define _DRAW_H

#include "isp.h"
#include "analysis.h"

#include <stdint.h>
#include <stdbool.h>

void drawImage(uint8_t * imageBuf, ImagePair *imagePair, ImageStats *statsH, ImageStats *statsV);

#endif // _DRAW_H
