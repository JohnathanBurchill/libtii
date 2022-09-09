/*

    TIIM processing library: lib/tiigraphics/video.c

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

#include "video.h"

#include "tiigraphics.h"
#include "colors.h"
#include "colortable.h"
#include "fonts.h"
#include "draw.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>


// consider -movflags +faststart
// -crf 17 or 18 (nominal is 23)
// -pix_fmt yuv420p for quicktime
// Use libx264rgb for RGB to avoid artifacts associated with YUV conversion
// -preset slow gives better quality for a given crf
// -tune stillimage?

// static ... here constrains global variables to the scope of this file 
static AVFormatContext *videoContext = NULL;
static AVDictionary *dict = NULL;
static AVStream *videoStream;
static AVFrame *videoFrame = NULL;
static AVPacket *videoPacket = NULL;

static const AVCodec *codec = NULL;
static AVCodecContext *codecContext = NULL;

static struct SwsContext *colorConversionContext = NULL;
static uint8_t frameBuffer[3*IMAGE_BUFFER_SIZE];

int initVideo(const char * videofilename)
{
    int status;

    // Try to be quiet
    av_log_set_level(AV_LOG_FATAL);

    videoFrame = av_frame_alloc();
    videoPacket = av_packet_alloc();
    av_dict_set(&dict, "profile", "baseline", 0);
    av_dict_set(&dict, "preset", "medium", 0);
    av_dict_set(&dict, "level", "3", 0);
    av_dict_set(&dict, "crf", "23", 0);
    av_dict_set(&dict, "tune", "grain", 0);
    av_dict_set(&dict, "loglevel", "quiet", 0);
    av_dict_set(&dict, "movflags", "faststart", 0);

    avformat_alloc_output_context2(&videoContext, NULL, NULL, videofilename);
    if (!videoContext)
    {
        fprintf(stderr, "Problem initializing output context\n");
        return VIDEO_OUTPUT_CONTEXT;
    }

    // Get the encoder
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        fprintf(stderr, "Problem finding the H.264 codec.\n");
        return VIDEO_NO_CODEC;
    }
    // Get a stream
    videoStream = avformat_new_stream(videoContext, NULL);
    if (!videoStream)
    {
        fprintf(stderr, "Problem initializing output stream\n");
        return VIDEO_OUTPUT_STREAM;
    }
    videoStream->id = 0;    

    // Get packet
    videoPacket = av_packet_alloc();
    if (!videoPacket)
    {
        fprintf(stderr, "Problem allocating packet.\n");
        return VIDEO_NO_PACKET;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        fprintf(stderr, "Problem allocating encoder context.\n");
        return VIDEO_NO_ENCODER_CONTEXT;
    }
    codecContext->codec_id = AV_CODEC_ID_H264;
    codecContext->bit_rate = 1000000;
    codecContext->width = IMAGE_WIDTH;
    codecContext->height = IMAGE_HEIGHT;
    videoStream->time_base = (AVRational){1, VIDEO_FPS};
    codecContext->time_base = videoStream->time_base;
    codecContext->gop_size = 250;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (videoContext->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    status = avcodec_open2(codecContext, codec, &dict);
    if (status < 0)
    {
        fprintf(stderr, "Problem opening codec.\n");
        return VIDEO_CODEC_OPEN;
    }

    videoFrame = av_frame_alloc();
    if (!videoFrame)
    {
        fprintf(stderr, "Problem allocating video frame.\n");
        return VIDEO_NO_FRAME;
    }
    videoFrame->format = codecContext->pix_fmt;
    videoFrame->width = codecContext->width;
    videoFrame->height = codecContext->height;
    status = av_frame_get_buffer(videoFrame, 0);
    if (status < 0)
    {
        fprintf(stderr, "Problem getting video frame buffer.\n");
        return status;
    }

    status = avcodec_parameters_from_context(videoStream->codecpar, codecContext);
    if (status < 0)
    {
        fprintf(stderr, "Problem copying stream parameters.\n");
        return VIDEO_STREAM_PARAMETERS;
    }

    status = avio_open(&videoContext->pb, videofilename, AVIO_FLAG_WRITE);
    if (status < 0)
    {
        fprintf(stderr, "Problem opening video file for writing\n");
        char err[255];
        av_strerror(status, err, 255);
        fprintf(stderr, "Message: %s\n", err);
        return status;
    }

    status = avformat_write_header(videoContext, &dict);
    if (status < 0)
    {
        fprintf(stderr, "Problem writing video header.\n");
        return status;
    }

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


    return VIDEO_OK;

}

static void rgbToYuv(uint8_t *rgb) {
    const int in_linesize[1] = { 3 * videoFrame->width };
    colorConversionContext = sws_getCachedContext(colorConversionContext,
            videoFrame->width, videoFrame->height, AV_PIX_FMT_RGB24,
            videoFrame->width, videoFrame->height, AV_PIX_FMT_YUV420P,
            0, NULL, NULL, NULL);
    sws_scale(colorConversionContext, (const uint8_t * const *)&rgb, in_linesize, 0,
            videoFrame->height, videoFrame->data, videoFrame->linesize);
}

int generateFrame(Image *image, int frameNumber)
{
    int status;
    int got_output;
    int color;
    if (image != NULL)
    {
        for (int i = 0; i < image->numberOfBytes; i++)
        {
            color = image->pixels[i];
            frameBuffer[3*i + 0] = colorsrgbrgb[3*color];
            frameBuffer[3*i + 1] = colorsrgbrgb[3*color+1];
            frameBuffer[3*i + 2] = colorsrgbrgb[3*color+2];
        }
        rgbToYuv(frameBuffer);
        videoFrame->pts = frameNumber;
        status = avcodec_send_frame(codecContext, videoFrame);
    }
    else 
    {
        status = avcodec_send_frame(codecContext, NULL);
    }

    if (status < 0)
    {
        fprintf(stderr, "Problem sending frame to encoder: %s\n", av_err2str(status));
        return VIDEO_FRAME_SEND;
    }
    while (status >= 0)
    {
        status = avcodec_receive_packet(codecContext, videoPacket);
        if (status == AVERROR(EAGAIN) || status == AVERROR_EOF)
        {
            break;
        }
        else if (status < 0)
        {
            fprintf(stderr, "Problem encoding frame: %s\n", av_err2str(status));
            return VIDEO_FRAME_ENCODE;
        }
        av_packet_rescale_ts(videoPacket, codecContext->time_base, videoStream->time_base);
        videoPacket->stream_index = videoStream->index;
        status = av_interleaved_write_frame(videoContext, videoPacket);
        if (status < 0)
        {
            fprintf(stderr, "Problem writing packet: %s\n", av_err2str(status));
            return VIDEO_FRAME_WRITE;
        }
    }
    return status;
}


int finishVideo(void)
{
    generateFrame(NULL, 0);
    av_write_trailer(videoContext);
    cleanupVideo();
    return VIDEO_OK;
}

void cleanupVideo(void)
{
    // free memory, contexts, etc.
    avcodec_free_context(&codecContext);
    av_frame_free(&videoFrame);
    av_packet_free(&videoPacket);
    sws_freeContext(colorConversionContext);
    avio_closep(&videoContext->pb);
    avformat_free_context(videoContext);
    return;
}


// Transitions
void insertTransition(Image *imageBuf, const char *text, int x0, int y0, int fontSize, double durationSeconds, int *frameCounter)
{
    drawFill(imageBuf, BACKGROUND_COLOR);
    annotate(text, fontSize, x0 - (int)((double)strlen(text)/2.*(double)fontSize/12.*8.), y0, imageBuf);
    for (int c = 0; c < (int)(durationSeconds*VIDEO_FPS); c++)
    {
        generateFrame(imageBuf, (*frameCounter)++);
    }
    drawFill(imageBuf, BACKGROUND_COLOR);
}
