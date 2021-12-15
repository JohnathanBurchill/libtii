#include "png.h"

#include <spng.h>
#include <errno.h>

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
        return status;
    }
    if (enc != NULL) spng_ctx_free(enc);
    return 0;
}