/*

    TIIM processing library: lib/tii/detector.c

    Copyright (C) 2023  Johnathan K Burchill

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


#include "detector.h"

#include "isp.h"

#include <stdio.h>
#include <math.h>

// Moved from TRACIS...
void detectorCoordinates(char satellite, int sensor, float *xc, float *yc)
{
    float x0 = 0.0;
    float y0 = 0.0;

    switch (satellite)
    {
        case 'A':
            if (sensor == H_SENSOR)
            {
                x0 = 33.0411;
                y0 = 31.1261;
            }
            else
            {
                x0 = 32.3852;
                y0 = 33.5174;
            }
            break;
        case 'B':
            if (sensor == H_SENSOR)
            {
                x0 = 33.8073;
                y0 = 32.496;
            }
            else
            {
                x0 = 33.8465;
                y0 = 33.3973;
            }
            break;
        case 'C':
            if (sensor == H_SENSOR)
            {
                x0 = 33.429;
                y0 = 29.2934;
            }
            else
            {
                x0 = 31.7489;
                y0 = 35.8786;
            }
            break;
        default:
            fprintf(stderr, "Invalid satellite in detectorCoordinates().\n");
            return;
    }

    if (xc != NULL)
        *xc = x0;

    if (yc != NULL)
        *yc = y0;

    return;
}

float eofr(double r, float innerDomeVoltage, float mcpVoltage)
{
    // Passing r by casting as double to avoid
    // making a temporary variable for r^3 calculations in double precision
    
    // innerDomeVoltage is voltage after subtracting faceplate voltage
    // i.e., the voltage between the domes

    // Model values from "Eofr.nb simulations with CEFI XTracer (TRACIS github)."
    static const double a00 =  0.008727536876620966;
    static const double a10 = -0.004454815158800095;
    static const double a20 =  0.0009085603706168588;
    static const double a30 = -0.000014365393160422022;
    static const double a01 = -0.010918278041742287;
    static const double a11 =  0.004981990763549952;
    static const double a21 = -0.00029784411180069903;
    static const double a31 =  8.680704266427428e-6;

    double referenceVoltage = -2400.0;
    double vmcpN = mcpVoltage / referenceVoltage;

    double energy = a00 + a10*r + a20*r*r + a30*r*r*r;
    energy += a01*vmcpN + a11*r*vmcpN + a21*r*r*vmcpN + a31*r*r*r*vmcpN;

    energy *= fabs(innerDomeVoltage);

    return (float)energy;
}

