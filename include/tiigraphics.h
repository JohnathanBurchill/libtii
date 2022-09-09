/*

    TIIM processing library: include/tiigraphics.h

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

#ifndef _TIIGRAPHICS_H
#define _TIIGRAPHICS_H

// 1920/1080 1/2 scale, correct proportion
#define IMAGE_WIDTH 960
#define IMAGE_HEIGHT 540
#define IMAGE_BUFFER_SIZE ((size_t)(IMAGE_WIDTH*IMAGE_HEIGHT))
#define VIDEO_FPS 15

#define RAW_IMAGE_OFFSET_X 15
#define RAW_IMAGE_OFFSET_Y 30
#define RAW_IMAGE_SCALE 3
#define RAW_IMAGE_SEPARATION_X 18

#define PA_REGION_IMAGE_OFFSET_X 670
#define PA_REGION_IMAGE_OFFSET_Y 10

#define PA_REGION_IMAGE_SCALE 2

#define GAIN_CORRECTED_IMAGE_SCALE 3
#define GAIN_CORRECTED_OFFSET_X RAW_IMAGE_OFFSET_X
#define GAIN_CORRECTED_OFFSET_Y RAW_IMAGE_OFFSET_Y + 235

#define LINE_SPACING 16

#define PA_TEXT_X 0
#define PA_TEXT_Y 100

#define MONITOR_LABEL_OFFSET_X 400
#define MONITOR_LABEL_OFFSET_Y 35


#endif // _TIIGRAPHICS_H
