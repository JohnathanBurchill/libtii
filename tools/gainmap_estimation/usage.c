/*

    TIIM processing tools: tools/gainmap_estimation/usage.c

    Copyright (C) 2024  Johnathan K Burchill

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
#include "usage.h"
#include "gainmap_estimation.h"

#include <stdio.h>

void usage(const char * name)
{
    printf("\nTII gain map estimation version %s compiled %s %s UTC\n", GAINMAP_ESTIMATION_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2023 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s <X> <yyyymmdd1> <yyymmdd2> <outputDir> <cropGeometryFile> [options]\n", name);
    printf("\n");
    printf("<X> designates the Swarm satellite (A, B or C). Must be run from directory containing EFIXTIC L0 files.\n");
    printf("<cropGeometryFile> contains deltas for each sensor's cropping ellispe position and dimensions.\n");
    printf("Options:\n");
    printf("  %30s - %s\n", "--gainmaps", "lists gainmap IDs and the dates they were loaded to the instrument");
    printf("  %30s - %s\n", "--previous-gainmap-voltage-steps", "Print the indices of the voltage steps used for the previous gain correction maps.\n");

    printf("  %30s - %s\n", "--minimum-image-maximum-<sensor>=<intValue>", "Include only images for sensor <sensor> (H or V) that have a maximum pixel value of at least <intVal>.\n");
    printf("  %30s - %s\n", "--sensor=<H or V>", "Select sensor for printing info and pixel values. Default is H.");
    printf("  %30s - %s\n", "--print-image-info", "Print voltages and number of images for each voltage step.");
    printf("  %30s - %s\n", "--print-image=<voltageIndex>", "Print image pixel averages for voltage step <voltageIndex>.");
    printf("  %30s - %s\n", "--csv=<type>", "Export image pixel values as CSV with floats represented in hexadecimal. <type> can be \"<stepIndexH>,<stepIndexV>\" or \"last\".");
    printf("  %30s - %s\n", "--png=<type>", "Save PNG of map. <type> can be \"<stepIndexH>,<stepIndexV>\", \"all\", or \"last\".\n");
    printf("  %30s - %s\n", "--raw-image", "raw image pixel averages.");
    printf("  %30s - %s\n", "--crop", "Apply crop.");
    printf("  %30s - %s\n", "--max<sensor>=<maxval>", "PNG color scale maximum for sensor <sensor> (H or V). Default: -1 (autoscale).");

    printf("  %30s - %s\n", "--crop-dx-<sensor>=<value>", "Crop mask x offset for sensor <sensor> (H or V). Default 0.");
    printf("  %30s - %s\n", "--crop-dy-<sensor>=<value>", "Crop mask y offset for sensor <sensor> (H or V). Default 0.");
    printf("  %30s - %s\n", "--crop-drx-<sensor>=<value>", "Crop mask rx offset for sensor <sensor> (H or V). Default 0.");
    printf("  %30s - %s\n", "--crop-dry-<sensor>=<value>", "Crop mask ry offset for sensor <sensor> (H or V). Default 0.");
    printf("  %30s - %s\n", "--min-bias=<value>", "Minimum voltage for inner dome bias. Default: -19.0");
    printf("  %30s - %s\n", "--max-bias=<value>", "Maximum voltage for inner dome bias. Default: 0.0");
    printf("  %30s - %s\n", "--delta-bias=<value>", "Inner dome bias step size. Default: -0.5");

    return;
}

