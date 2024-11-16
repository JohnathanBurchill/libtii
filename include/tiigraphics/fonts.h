/*

    TIIM processing library: include/fonts.h

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

#ifndef _FONTS_H
#define _FONTS_H

#include "draw.h"

#include <stdint.h>


#define FONTLEVEL1 16
#define FONTLEVEL2 93
#define FONTLEVEL3 200


enum FONTS_ERR {
    FONTS_OK = 0,
    FONTS_FONTSIZE_UNAVAILABLE = -1,
    FONTS_CHARACTER_UNAVAILABLE = -2,
    FONTS_FONT_FILE_ISSUE = -3
};

int annotate(const char * text, int fontsize, int x, int y, Image *imageBuffer);
int placeCharacter(uint8_t c, int8_t *font, int x, int y, Image *imageBuffer);


#endif // _FONTS_H
