#ifndef _TIIM_H
#define _TIIM_H

#include <libxml/xpathInternals.h>

#define TIIM_VERSION "0101"

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define IMAGE_BUFFER_SIZE ((size_t)(IMAGE_WIDTH*IMAGE_HEIGHT))

#define IMAGE_OFFSET_X 30
#define IMAGE_OFFSET_Y 40
#define V_IMAGE_OFFSET_X 45

#define PA_REGION_IMAGE_OFFSET_X 30
#define PA_REGION_IMAGE_OFFSET_Y 300

#define RAW_IMAGE_SCALE 3
#define PA_REGION_IMAGE_SCALE 2
#define GAIN_CORRECTED_IMAGE_SCALE 2

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
#define PA_MINIMUM_VALUE 50

#define MONITOR_LABEL_OFFSET_X 390
#define MONITOR_LABEL_OFFSET_Y 30

int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _TIIM_H

