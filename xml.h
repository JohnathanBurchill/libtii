#ifndef _XML_H
#define _XML_H

#include <libxml/parser.h>

int getLongValue(xmlDocPtr doc, const char * xpath, long *value);

#endif // _XML_H
