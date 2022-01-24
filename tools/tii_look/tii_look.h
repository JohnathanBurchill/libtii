#ifndef _TII_LOOK_H
#define _TII_LOOK_H

#include "draw.h"

#define LOOK_IMAGE_WIDTH 1024
#define LOOK_IMAGE_HEIGHT 768
#define LOOK_IMAGE_BYTES_PER_PIXEL 1

#define LOOK_IMAGE_SCALE 4
#define LOOK_IMAGE_RAW_X0 30
#define LOOK_IMAGE_RAW_Y0 50
#define LOOK_IMAGE_GC_X0 530
#define LOOK_IMAGE_GC_Y0 LOOK_IMAGE_RAW_Y0
#define LOOK_IMAGE_SEPARATION 20

void imagePoint(Image *image, bool correctedImage, int sensor, int columnFromLeft, int rowFromTop, int color, int size);

void imageVerticalLine(Image *image, bool correctedImage, int sensor, int columnFromLeft, int startRowFromTop, int stopRowFromTop, int color, int size);

void imageHorizontalLine(Image *image, bool correctedImage, int sensor, int startColumnFromLeft, int stopColumnFromLeft, int rowFromTop, int color, int size);

void imageRectangle(Image *image, bool correctedImage, int sensor, int startColumnFromLeft, int stopColumnFromLeft, int startRowFromTop, int stopRowFromTop, int color);

void imageOrigin(bool correctedImage, int sensor, int *x0, int *y0);

#endif // _TII_LOOK_H
