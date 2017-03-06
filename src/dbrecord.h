/***
 *
 * dbrecord.h - DBRecord structure
 * The internal database record is defined here.
 *
 * $Id: dbrecord.h,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#ifndef _DBRECORD_H
#define _DBRECORD_H

#include <stdint.h>
#include <time.h>

typedef struct
{
    // Primary key
    time_t   time;
    
    // Wind data
    uint16_t windDir;
    float    windGust;
    float    windAvrg;
    uint8_t  windChill;

    // Rain data
    uint16_t rainCurrent;
    float    rainTotal;
    time_t   rainTotalStart;
    uint16_t rainYesterday;

    // Channel data
    float    channelTemp[3];
    uint8_t  channelHumidity[3];
    uint8_t  channelDewTemp[3];

    // Mushroom data
    float    mushroomTemp;
    uint8_t  mushroomHumidity;
    uint8_t  mushroomDewTemp;

    // BTH data
    float    bthTemp;
    uint8_t  bthHumidity;
    uint8_t  bthDewTemp;
    uint16_t bthBaro;
    char     bthWeatherStatus[32];

    // EXTBTH data
    float    extbthTemp;
    uint8_t  extbthHumidity;
    uint8_t  extbthDewTemp;
    uint16_t extbthBaro;
    char     extbthWeatherStatus[32];
} DBRecord;

#endif // _DBRECORD_H
