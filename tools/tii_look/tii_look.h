/*

    TIIM processing tools: tools/tii_look/tii_look.h

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
