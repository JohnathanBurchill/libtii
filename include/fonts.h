#ifndef _FONTS_H
#define _FONTS_H

#include <stdint.h>


#define FONTLEVEL1 16
#define FONTLEVEL2 93
#define FONTLEVEL3 200


int annotate(const char * text, int fontsize, int x, int y, uint8_t *imageBuffer);
int placeCharacter(uint8_t c, int8_t *font, int x, int y, uint8_t *imageBuffer);

enum FONTS_ERR {
    FONTS_OK = 0,
    FONTS_FONTSIZE_UNAVAILABLE = -1,
    FONTS_CHARACTER_UNAVAILABLE = -2,
    FONTS_FONT_FILE_ISSUE = -3
};

#endif // _FONTS_H
