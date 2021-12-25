#include "utility.h"

#include "isp.h"
#include "tiim.h"

#include <stdio.h>
#include <string.h>

int constructMovieFilename(ImagePackets *imagePackets, ImagePair *imagePair, const char *outputDir, char *movieFilename)
{

    char satellite;
    IspDateTime * date = NULL;
    char startDate[16];
    char stopDate[16];
    memset(startDate, 0, 16);
    memset(stopDate, 0, 16);

    getFirstImagePair(imagePackets, imagePair);

    satellite = getSatellite(imagePair);
    if (satellite == 'X')
        return UTIL_MOVIE_NAME_NO_SATELLITE;

    // This is like the unix time functions -> the returned pointer is valid until the next call.
    date = getIspDateTime(imagePair);
    if (date == NULL)
        return UTIL_MOVIE_NAME_NO_DATE;

    sprintf(startDate, "%04d%02d%02dT%02d%02d%02d", date->year, date->month, date->day, date->hour, date->minute, date->second);

    getLastImagePair(imagePackets, imagePair);
    date = getIspDateTime(imagePair);
    sprintf(stopDate, "%04d%02d%02dT%02d%02d%02d", date->year, date->month, date->day, date->hour, date->minute, date->second);

    sprintf(movieFilename, "%s/SW_OPER_EFI%cTIIMOV_%s_%s_%s.mp4", outputDir, satellite, startDate, stopDate, TIIM_VERSION);

    return UTIL_OK;

}
