#ifndef _UTILITY_H
#define _UTILITY_H

#include "isp.h"

enum UTIL_CODES
{
    UTIL_OK = 0,
    UTIL_MOVIE_NAME_NO_DATE = -1,
    UTIL_MOVIE_NAME_NO_SATELLITE = -2
};

int constructMovieFilename(ImagePackets *imagePackets, ImagePair *imagePair, char *movieFilename);

#endif // _UTILITY_H