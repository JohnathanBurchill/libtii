#ifndef _TIIM_H
#define _TIIM_H

#include <libxml/xpathInternals.h>

#define TIIM_VERSION "0101"

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
#define PA_REGION_IMAGE_OFFSET_Y 40

#define PA_REGION_IMAGE_SCALE 2

#define GAIN_CORRECTED_IMAGE_SCALE 3
#define GAIN_CORRECTED_OFFSET_X RAW_IMAGE_OFFSET_X
#define GAIN_CORRECTED_OFFSET_Y RAW_IMAGE_OFFSET_Y + 235

#define FOREGROUND_COLOR 252 // three font colors, with index 252 being the darkest
#define BACKGROUND_COLOR 255
#define MIN_COLOR_VALUE 0
#define MAX_COLOR_VALUE 251

#define LINE_SPACING 16

#define MAX_PIXEL_VALUE_FOR_MAX_CALCULATION 2000
#define OPTICAL_CENTER_X 36.0
#define OPTICAL_CENTER_Y 32.5

#define PA_MINIMUM_RADIUS 29.0
#define PA_MAXIMUM_RADIUS 38.0
#define PA_DY_FACTOR 0.7
#define PA_MINIMUM_VALUE 100
#define PA_ANGULAR_BIN_WIDTH 15
#define PA_ANGULAR_BIN_START 60.0
#define PA_ANGULAR_BIN_STOP 330.0
#define PA_ANGULAR_NUM_BINS 18
//#define PA_ANGULAR_NUM_BINS ((int)((PA_ANGULAR_BIN_STOP - PA_ANGULAR_BIN_START)/PA_ANGULAR_BIN_WIDTH))
#define PA_TEXT_X 0
#define PA_TEXT_Y 100

#define MONITOR_LABEL_OFFSET_X 400
#define MONITOR_LABEL_OFFSET_Y 35

int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _TIIM_H

