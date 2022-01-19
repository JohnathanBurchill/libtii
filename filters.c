#include "filters.h"

#include "tiim.h"
#include "analysis.h"

#include <stdint.h>
#include <stdarg.h>
#include <math.h>

// Image Filters
// All must return double

double identityFilter(int k, void *pixelBuf, bool* missing, void* args)
{
    if (missing != NULL)
        *missing = false;
    return (double) ((uint16_t*)pixelBuf)[k];
}

double paAngularSpectrumFilter(int k, void *pixelBuf, bool *missing, void *args)
{
    double v = 0;
    int x, y;
    double dx, dy, r, r1, phidx, phidy, phi;
    int paBin;
    
    x = k / TII_ROWS;
    y = (TII_ROWS-1) - (k % TII_ROWS);
    dx = (double) x - OPTICAL_CENTER_X;
    dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
    r = hypot(dx, dy);
    r1 = hypot(dx, dy / PA_DY_FACTOR);
    phidx = (double) x - PA_ANGULAR_X0;
    phidy = PA_ANGULAR_Y0 - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
    phi = atan2(phidy, phidx) * 180.0 / M_PI;
    paBin = getPaBin(phi);

    int *spectrum = (int*) args;
    if (paBin >=0 && paBin < PA_ANGULAR_NUM_BINS)
        v = (double)spectrum[paBin];
    if (missing != NULL)
    {
        if (!(r1 >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS))
            *missing = true;
        else
            *missing = false;
    }
        
    
    return v;
}