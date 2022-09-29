/*

    TIIM processing library: lib/tii/detector.c

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


// Moved from TRACIS...
float eofr(double r, float innerDomeVoltage, float mcpVoltage)
{
    // Passing r by casting as double to avoid
    // making a temporary variable for r^3 calculation in double precision
    // TODO incorporate MCP voltage into transfer function

    double a = 0.0, b = 0.0, c = 0.0, d = 0.0;

    float energy = 0.0;

    // Model values from "Along-track-drift-anomaly.nb simulations with CEFI XTracer."

    if (innerDomeVoltage < -90.0) // -99.0 V simulation, need to update for actual voltages
    {
        a = -0.9849666707472252;
        b = 0.25076488696479904;
        c = 0.02909150117625129;
        d = 0.00017439906324331827;
    }
    else // Simulated for -60.3 V, but will apply to any other voltage until covered by new simulations.
    {
        a = -2.4014615344450805;
        b = 0.7439055999344776;
        c = -0.030922802815747417;
        d = 0.001209654508105981;
    }

    energy = (float) (a + b*r + c*r*r + d*r*r*r);

    if (innerDomeVoltage > -60.) // Scale by inner dome voltage until new simulations are done
    {
        energy = energy / 60.3 * fabs(innerDomeVoltage);
    }

    return energy;
}

