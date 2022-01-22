#include "anomaly_stats.h"

#include "tii.h"

#include "isp.h"
#include "import.h"
#include "utility.h"
#include "analysis.h"
#include "timeseries.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        statsusage(argv[0]);
        exit(1);
    }

    int status = 0;

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);

    // Daily imaging anomaly statistics only
    if (sourceLen != 9)
    {
        statsusage(argv[0]);
	    exit(1);
    }
    double max = -1.0;
    char *outputDir = argv[2];

    // Data
    ImagePackets imagePackets;

    status = importImagery(satDate, &imagePackets);
    if (status)
    {
        fprintf(stderr, "Could not import image data.\n");
        goto cleanup;
    }
    if (imagePackets.numberOfImages == 0)
    {
        fprintf(stderr, "No images found for satellite %c on %s\n", satDate[0], satDate+1);
        goto cleanup;
    }
    
    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS], pixelsV[NUM_FULL_IMAGE_PIXELS];
    FullImagePacket * fip1, *fip2;
    FullImageContinuedPacket *cip1, *cip2;
    ImagePair imagePair;
    ImageAuxData auxH, auxV;
    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    getFirstImagePair(&imagePackets, &imagePair);
    double dayStart = imagePair.secondsSince1970;
    char satellite = getSatellite(&imagePair);
    getLastImagePair(&imagePackets, &imagePair);
    double dayEnd = imagePair.secondsSince1970;
    // Filter images based on time of day if yyyymmdd was passed
    // Seconds since 1970
    if (dateToSecondsSince1970(satDate + 1, &dayStart))
    {
        fprintf(stderr, "Could not parse %s to a date.\n", satDate);
        goto cleanup;
    }
    dayEnd = dayStart + 86400.0; // ignore leap second on this day
 
    size_t numberOfImagePairs = countImagePairs(&imagePackets, &imagePair, dayStart, dayEnd);

    // Per image measles and PA stats
    char dailyAnomalyFilename[FILENAME_MAX];
    sprintf(dailyAnomalyFilename, "%s/SW_EFI%s_imaging_anomaly_stats.txt", outputDir, satDate);
    FILE *dailyAnomalyFile = fopen(dailyAnomalyFilename, "w");
    if (dailyAnomalyFile == NULL)
    {
        fprintf(stderr, "Could not open imaging anomaly stats file.\n");
        goto cleanup;
    }

    ImageAnomalies h;
    ImageAnomalies v;

    for (size_t i = 0; i < numberOfImagePairs; i++)
    {
        getImagePair(&imagePackets, i, &imagePair);

        if (scienceMode(imagePair.auxH) && scienceMode(imagePair.auxV))
        {
            analyzeImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
            analyzeImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

            // time in sec since 1970, PA H, PA V, measles H, measles V 
            fprintf(dailyAnomalyFile, "%ld %d %d %d %d\n", (time_t)floor(imagePair.secondsSince1970), h.peripheralAnomaly, v.peripheralAnomaly, h.measlesAnomaly, v.measlesAnomaly);
        }
    }
    fclose(dailyAnomalyFile);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);

    fflush(stdout);

    exit(0);
}



void analyzeImageAnomalies(uint16_t *pixels, bool gotImage, char satellite, ImageAnomalies *anomalies)
{
    initializeAnomalyData(anomalies);
    size_t measlesCounter = 0;
    size_t paCounter = 0;

    int x, y; // pixel coordinates
    double dx, dy, r, phidx, phidy, phi;
    int paBin;
    if (gotImage)
    {
        // Measles, PA
        for (int k = 0; k < NUM_FULL_IMAGE_PIXELS; k++)
        {
            if (pixels[k] == 4095)
                measlesCounter++;

            x = k / TII_ROWS;
            y = (TII_ROWS - 1) - (k % TII_ROWS);
            dx = (double) x - OPTICAL_CENTER_X;
            dy = OPTICAL_CENTER_Y - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
            r = hypot(dx, dy);
            phidx = (double) x - PA_ANGULAR_X0;
            phidy = PA_ANGULAR_X0 - (double) y; // y increases downward, switch to match graphics in case needed for other analysis
            phi = atan2(phidy, phidx) * 180.0 / M_PI;
            if (r >= PA_MINIMUM_RADIUS && r <= PA_MAXIMUM_RADIUS && pixels[k] != 4095 && pixels[k] > PA_MINIMUM_VALUE)
                paCounter++;
        }
        if (measlesCounter >= 10)
            anomalies->measlesAnomaly = true;

        if (paCounter > 10)
            anomalies->peripheralAnomaly = true;


        // Get first moment
        // Calculate 2nd y moment in box 15x15 box around 1st moment if this is the O+ signal
        // classic wing anomaly
        // if 2nd y moment > 5? Only works for gain-corrected images

        // Bifurcation anomaly
        // Get pixel counts at sqrt(2ndy moment) above and below y1
        // If those values are both larger than pixel count at y1, bifurcation is probable

        // upper angel's wing anomaly
        // Take a vertical line of pixels at upper right
        // If we have counts all along the line, upper angel's wing

        // lower angel's wing anomaly
        // Take a vertical line of pixels at lower right
        // If we have counts all along the line, lower angel's wing

        // Ring anomaly
        // Sum pixels in five regions around an arc at left of image. 
        // If all regions have counts greater than some threshold
        // and central region has higher count than any other
        // and each adjacent region has higher count than the adjacent arc tip regions
        // class ring anomaly 

    }

    return;

}

void initializeAnomalyData(ImageAnomalies *anomalies)
{
    anomalies->peripheralAnomaly = false;
    anomalies->upperAngelsWingAnomaly = false;
    anomalies->lowerAngelsWingAnomaly = false;
    anomalies->classicWingAnomaly = false;
    anomalies->bifurcationAnomaly = false;
    anomalies->ringAnomaly = false;
    anomalies->measlesAnomaly = false;

    return;
}

bool scienceMode(ImageAuxData *aux)
{
    // Unable to check AGC from image ISPs. Should be on for science mode, but will allow non-AGC ops.
    // MCP voltage less than -1000 V
    // Phosphor voltage > 3900 V
    // Bias voltage < -50 V
    return aux->McpVoltageMonitor < -1000.0 && aux->PhosphorVoltageMonitor > 3900 && aux->BiasGridVoltageMonitor < -50.0;

}

void statsusage(const char * name)
{
    printf("\nTII Daily Image Anomaly Statistics %s compiled %s %s UTC\n", TII_LIB_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");

    return;
}

