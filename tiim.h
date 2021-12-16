#ifndef _TIIM_H
#define _TIIM_H

#include <libxml/xpathInternals.h>

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define IMAGE_BUFFER_SIZE ((size_t)(IMAGE_WIDTH*IMAGE_HEIGHT))

#define IMAGE_OFFSET_X 30
#define IMAGE_OFFSET_Y 40
#define V_IMAGE_OFFSET_X 45

#define IMAGE_SCALE 3

int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _TIIM_H

