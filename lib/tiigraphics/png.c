#include "png.h"

#include "tiigraphics.h"
#include "colortable.h"
#include "fonts.h"
#include "spng.h"

#include <errno.h>

struct spng_plte getColorTable()
{
    struct spng_plte colorTable;
    colorTable.n_entries = 256;
    for (int c = MIN_COLOR_VALUE; c <= MAX_COLOR_VALUE; c++)
    {
        colorTable.entries[c].red = colorsrgbrgb[3*c];
        colorTable.entries[c].green = colorsrgbrgb[3*c + 1];
        colorTable.entries[c].blue = colorsrgbrgb[3*c + 2];
    }
    // Foreground black
    colorTable.entries[252].red = FONTLEVEL1;
    colorTable.entries[252].green = FONTLEVEL1;
    colorTable.entries[252].blue = FONTLEVEL1;
    colorTable.entries[253].red = FONTLEVEL2;
    colorTable.entries[253].green = FONTLEVEL2;
    colorTable.entries[253].blue = FONTLEVEL2;
    colorTable.entries[254].red = FONTLEVEL3;
    colorTable.entries[254].green = FONTLEVEL3;
    colorTable.entries[254].blue = FONTLEVEL3;
    // Background white / transparent
    colorTable.entries[BACKGROUND_COLOR].red = 255;
    colorTable.entries[BACKGROUND_COLOR].green = 255;
    colorTable.entries[BACKGROUND_COLOR].blue = 255;
    colorTable.entries[BACKGROUND_COLOR].alpha = 0;

    return colorTable;

}

int writePng(const char * filename, uint8_t *pixels, int width, int height, struct spng_plte *colorTable)
{
    int status = 0;
    // Make png files with text overlays
    FILE *pngFile = fopen(filename, "w");
    if (pngFile == NULL)
    {
        return errno;
    }
    spng_ctx *enc = spng_ctx_new(SPNG_CTX_ENCODER);
    spng_set_png_file(enc, pngFile);

    struct spng_ihdr ihdr;
    ihdr.bit_depth = 8;
    ihdr.color_type = SPNG_COLOR_TYPE_INDEXED;
    ihdr.compression_method = 0;
    ihdr.height = height;
    ihdr.width = width;
    ihdr.interlace_method = SPNG_INTERLACE_NONE;
    ihdr.filter_method = SPNG_FILTER_NONE;
    spng_set_ihdr(enc, &ihdr);
    spng_set_plte(enc, colorTable);
    struct spng_bkgd background;
    background.plte_index = 255;
    spng_set_bkgd(enc, &background);

    size_t out_size = width*height;
    status = spng_encode_image(enc, pixels, out_size, SPNG_FMT_RAW, SPNG_ENCODE_FINALIZE);
    if (status)
    {
        printf("spng_encode_image() error: %s\n", spng_strerror(status));
    }
    if (enc != NULL) spng_ctx_free(enc);

    fclose(pngFile);
    return status;
}
