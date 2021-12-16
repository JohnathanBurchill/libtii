#include "fonts.h"
#include "tiim.h"

#include "fonts9_to_24.h"

#include <stdint.h>
#include <string.h>

// Draw text in the image buffer
int annotate(const char * text, int fontsize, int x, int y, uint8_t *imageBuffer)
{
    int status = FONTS_OK;
    int8_t *font;
    int fontWidth = 8 * fontsize / 12;
    int fontHeight = 16 * fontsize / 12;
    switch (fontsize)
    {
        case 9:
            font = font9;
            break;
        case 12:
            font = font12;
            break;
        case 15:
            font = font15;
            break;
        case 18:
            font = font18;
            break;
        case 21:
            font = font21;
            break;
        case 24:
            font = font24;
            break;
        default:
            printf("Just stick to the available font sizes: 9 through 24 in steps of 3.\n");
            return FONTS_FONTSIZE_UNAVAILABLE;
            break;
    }

    int nCharacters = strlen(text);
    int xpos = x;
    int ypos = y;
    for (int i = 0; i < nCharacters; i++)
    {
        // "\n"?
        if (text[i] == 10)
        {
            ypos+=fontHeight;
            xpos = x;
        }
        status = placeCharacter(text[i], font, xpos, ypos, imageBuffer);
        if(status)
        {
            printf("I didn't anticipate that you would try to do that! I'm giving up trying to scribble on this image.");
            return status;
        }
        xpos += fontWidth;
    }

    return status;
}


int placeCharacter(uint8_t characterNumber, int8_t *font, int x, int y, uint8_t *imageBuffer)
{
    int status = FONTS_OK;

    if (characterNumber > 127)
    {
        return FONTS_CHARACTER_UNAVAILABLE;
    }

    int characterIndex = 0;
    int positionIndex = 0;
    int xoffset, yoffset;
    int fontColorIndex;
    int imageIndex;
    while (characterIndex < characterNumber -1)
    {
        if (font[positionIndex++] == -1)
            characterIndex++;
    }
    // Found the begining of the character position codes
    while(font[positionIndex] != -1)
    {
        yoffset = font[positionIndex++];
        xoffset = font[positionIndex++];
        fontColorIndex = font[positionIndex++];
        if (xoffset == -1 || yoffset == -1 || fontColorIndex == -1)
        {
            // Font offsets not paired 
            status = FONTS_FONT_FILE_ISSUE;
            break;
        }
        imageIndex = (IMAGE_WIDTH*(y + yoffset)+(x + xoffset));
        if (imageIndex >= 0 && imageIndex < IMAGE_BUFFER_SIZE)
        {
            // on image data just use darkest level
            if (imageBuffer[imageIndex] <= MAX_COLOR_VALUE)
            {
                // Otherwise the characters are too bold
                if (fontColorIndex < 3)
                   imageBuffer[imageIndex] = MAX_COLOR_VALUE + 1;
            }
            else
            {
                // On background (white)
                imageBuffer[imageIndex] = MAX_COLOR_VALUE + fontColorIndex;
            }
        }
    }

    return status;
}