#include "libavutil.h"
#include "tiim.h"
#include "colortable.h"
#include "fonts.h"

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

// Adapted from Stackoverflow
// https://stackoverflow.com/questions/12831761/how-to-resize-a-picture-using-ffmpegs-sws-scale/

static AVCodecContext *c = NULL;
static AVFrame *frame;
static AVPacket pkt;
static FILE *file;
static struct SwsContext *sws_context = NULL;
static uint8_t frameBuffer[3*IMAGE_BUFFER_SIZE];

static void ffmpeg_encoder_init_frame(AVFrame **framep, int width, int height) {
    int ret;
    AVFrame *frame;
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = width;
    frame->height = height;
    ret = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, frame->format, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }
    *framep = frame;
}

static void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t *rgb) {
    const int in_linesize[1] = { 3 * frame->width };
    sws_context = sws_getCachedContext(sws_context,
            frame->width, frame->height, AV_PIX_FMT_RGB24,
            frame->width, frame->height, AV_PIX_FMT_YUV420P,
            0, NULL, NULL, NULL);
    sws_scale(sws_context, (const uint8_t * const *)&rgb, in_linesize, 0,
            frame->height, frame->data, frame->linesize);
}

void ffmpeg_encoder_start(const char *filename, int codec_id, int fps, int width, int height, float factor) {
    AVCodec *codec;
    int ret;
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    c->bit_rate = 1000000;
    c->width = width;
    c->height = height;
    c->time_base.num = 1;
    c->time_base.den = fps;
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    ffmpeg_encoder_init_frame(&frame, width, height);

    // Set index color RGB values
    colorsrgbrgb[3*252] = FONTLEVEL1;
    colorsrgbrgb[3*252+1] = FONTLEVEL1;
    colorsrgbrgb[3*252+2] = FONTLEVEL1;
    colorsrgbrgb[3*253] = FONTLEVEL2;
    colorsrgbrgb[3*253+1] = FONTLEVEL2;
    colorsrgbrgb[3*253+2] = FONTLEVEL2;
    colorsrgbrgb[3*254] = FONTLEVEL3;
    colorsrgbrgb[3*254+1] = FONTLEVEL3;
    colorsrgbrgb[3*254+2] = FONTLEVEL3;
    // Background white / transparent
    colorsrgbrgb[3*255] = 255;
    colorsrgbrgb[3*255+1] = 255;
    colorsrgbrgb[3*255+2] = 255;

}

void ffmpeg_encoder_finish(void) {
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    int got_output, ret;
    do {
        fflush(stdout);
        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, file);
            av_packet_unref(&pkt);
        }
    } while (got_output);
    fwrite(endcode, 1, sizeof(endcode), file);
    fclose(file);
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
}

void ffmpeg_encoder_encode_frame(uint8_t *pixels, int pts) {
    int ret, got_output;
    int color;
    for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
    {
        color = pixels[i];
        frameBuffer[3*i + 0] = colorsrgbrgb[3*color];
        frameBuffer[3*i + 1] = colorsrgbrgb[3*color+1];
        frameBuffer[3*i + 2] = colorsrgbrgb[3*color+2];
    }
    ffmpeg_encoder_set_frame_yuv_from_rgb(frameBuffer);
    frame->pts = pts;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }
    if (got_output) {
        fwrite(pkt.data, 1, pkt.size, file);
        av_packet_unref(&pkt);
    }
}
