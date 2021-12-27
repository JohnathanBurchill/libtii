#ifndef _VIDEO_H
#define _VIDEO_H

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
int generateFrame(uint8_t *pixels, int frameNumber);
int finishVideo(void);
void cleanupVideo(void);

#endif // _VIDEO_H
