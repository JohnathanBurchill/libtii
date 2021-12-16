#ifndef _DRAW_H
#define _DRAW_H

#include "isp.h"
#include "analysis.h"

#include <stdint.h>
#include <stdbool.h>

void drawImage(uint8_t * imageBuf, ImageAuxData *aux1, uint16_t *pixels1, bool gotHImage, ImageStats *statsH, ImageAuxData *aux2, uint16_t *pixels2, bool gotVImage, ImageStats *statsV);

#endif // _DRAW_H
