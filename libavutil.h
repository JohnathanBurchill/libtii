#ifndef _LIBAVUTIL_H
#define _LIBAVUTIL_H

#include <libavcodec/avcodec.h>

static void ffmpeg_encoder_init_frame(AVFrame **framep, int width, int height);

static void ffmpeg_encoder_scale(uint8_t *rgb);

static void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t *rgb);

void ffmpeg_encoder_start(const char *filename, int codec_id, int fps, int width, int height, float factor);

void ffmpeg_encoder_finish(void);

void ffmpeg_encoder_encode_frame(uint8_t *rgb, int pts);

#endif // _LIBAVUTIL_H
