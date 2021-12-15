#ifndef _TIIM_H
#define _TIIM_H

#include <libxml/xpathInternals.h>

#define IMAGE_BUFFER_SIZE ((size_t)((2*40+20)*66))

int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _TIIM_H

