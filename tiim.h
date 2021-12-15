#ifndef _TIIM_H
#define _TIIM_H

#include <libxml/xpathInternals.h>

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define IMAGE_BUFFER_SIZE ((size_t)(IMAGE_WIDTH*IMAGE_HEIGHT))

#define IMAGE_OFFSET_X 50
#define IMAGE_OFFSET_Y 300
#define V_IMAGE_OFFSET_X 50

#define H_IMAGE_SCALE 2
#define V_IMAGE_SCALE

int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _TIIM_H

