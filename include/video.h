/*

    TIIM processing library: include/video.h

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

#ifndef _VIDEO_H
#define _VIDEO_H

#include <draw.h>

#include <stdint.h>

enum VIDEO_ERR {
    VIDEO_OK = 0,
    VIDEO_OUTPUT_CONTEXT = -1,
    VIDEO_OUTPUT_STREAM = -2,
    VIDEO_NO_CODEC = -3,
    VIDEO_NO_PACKET = -4,
    VIDEO_NO_ENCODER_CONTEXT = -5,
    VIDEO_CODEC_OPEN = -6,
    VIDEO_NO_FRAME = -7,
    VIDEO_STREAM_PARAMETERS = -8,
    VIDEO_FRAME_SEND = -9,
    VIDEO_FRAME_ENCODE = -10,
    VIDEO_FRAME_WRITE = -11

};

int initVideo(const char * filename);
int generateFrame(Image *image, int frameNumber);
int finishVideo(void);
void cleanupVideo(void);

// Video transitions
void insertTransition(Image *image, const char *text, int x0, int y0, int fontSize, double durationSeconds, int *frameCounter);

#endif // _VIDEO_H
