/***
 *
 * protocol.h - Packet structures and defines
 *              according to protocol
 *
 * $Id: protocol.h,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stdint.h>

enum
{
    // Packet lengths without checksum
    PL_WIND                  = 0x07,
    PL_RAIN                  = 0x0C,
    PL_THERMO_HYGRO          = 0x05,
    PL_MUSHROOM              = 0x05,
    PL_THERMO                = 0x03,
    PL_THERMO_HYGRO_BARO     = 0x09,
    PL_THERMO_HYGRO_BARO_EXT = 0x0A,
    PL_MINUTE                = 0x01,
    PL_CLOCK                 = 0x05,
    PL_MAX                   = 0x0C,
    // Weather status
    WS_SUNNY                 = 0x0C,
    WS_HALF_CLOUDY           = 0x06,
    WS_CLOUDY                = 0x02,
    WS_RAINY                 = 0x03,
    // Channels 
    CHANNEL1                 = 0x01,
    CHANNEL2                 = 0x02,
    CHANNEL3                 = 0x04,
};

// Type 0x00 (7 bytes)
typedef struct
{
    uint32_t __pad0        : 4;
    uint32_t gustOver      : 1;
    uint32_t avrgOver      : 1;
    uint32_t lowBatt       : 1;
    uint32_t __pad1        : 1;
    uint32_t dir_bcd       : 12; // BCD: 1|10|100
    uint32_t gustSpeed_bcd : 12; // BCD: 0.1|1|10
    uint32_t avrgSpeed_bcd : 12; // BCD: 0.1|1|10
    uint32_t __pad2        : 1;
    uint32_t chillNoData   : 1;
    uint32_t chillOver     : 1;
    uint32_t chill_sign    : 1;  // SIGN
    uint32_t chill_bcd     : 8;  // BCD: 1|10
} PacketWind;

// Type 0x01 (12 bytes)
typedef struct
{
    uint32_t __pad0          : 4;
    uint32_t rateOver        : 1;
    uint32_t totalOver       : 1;
    uint32_t lowBatt         : 1;
    uint32_t yestOver        : 1;
    uint32_t rate_bcd        : 12; // BCD: 1|10|100
    uint32_t total_bcd       : 20; // BCD: 0.1|1|10|100|1000
    uint32_t yest_bcd        : 16; // BCD: 1|10|100|1000
    uint32_t totalMinute_bcd : 8;  // BCD: 1|10
    uint32_t totalHour_bcd   : 8;  // BCD: 1|10
    uint32_t totalDay_bcd    : 8;  // BCD: 1|10
    uint32_t totalMonth_bcd  : 8;  // BCD: 1|10
    uint32_t totalYear_bcd   : 8;  // BCD: 1|10
} PacketRain;

// Type 0x02 (5 bytes)
typedef struct
{
    uint32_t channel     : 4;
    uint32_t dewUnder    : 1;
    uint32_t __pad0      : 1;
    uint32_t lowBatt     : 1;
    uint32_t __pad1      : 1;
    uint32_t temp_bcd    : 14; // BCD: 0.1|1|10|100
    uint32_t overUnder   : 1;
    uint32_t temp_sign   : 1;  // SIGN
    uint32_t hum_bcd     : 8;  // BCD: 1|10
    uint32_t dewTemp_bcd : 8;  // BCD: 1|10
} PacketThermoHygro;

// Type 0x03 (5 bytes)
typedef struct
{
    uint32_t __pad0      : 4;
    uint32_t dewUnder    : 1;
    uint32_t __pad1      : 1;
    uint32_t lowBatt     : 1;
    uint32_t __pad2      : 1;
    uint32_t temp_bcd    : 14; // BCD: 0.1|1|10|100
    uint32_t overUnder   : 1;
    uint32_t temp_sign   : 1;  // SIGN
    uint32_t hum_bcd     : 8;  // BCD: 1|10
    uint32_t dewTemp_bcd : 8;  // BCD: 1|10
} PacketMushroom;

// Type 0x04 (3 bytes)
typedef struct
{
    uint32_t channel   : 4;
    uint32_t __pad0    : 2;
    uint32_t lowBatt   : 1;
    uint32_t __pad1    : 1;
    uint32_t temp_bcd  : 14; // BCD: 0.1|1|10|100
    uint32_t overUnder : 1;
    uint32_t temp_sign : 1;  // SIGN
} PacketThermo;

// Type 0x05 (9 bytes)
typedef struct
{
    uint32_t __pad0        : 4;
    uint32_t dewUnder      : 1;
    uint32_t __pad1        : 1;
    uint32_t lowBatt       : 1;
    uint32_t __pad2        : 1;
    uint32_t temp_bcd      : 14; // BCD: 0.1|1|10|100
    uint32_t overUnder     : 1;
    uint32_t temp_sign     : 1;  // SIGN
    uint32_t hum_bcd       : 8;  // BCD: 1|10
    uint32_t dewTemp_bcd   : 8;  // BCD: 1|10
    uint32_t adcBaro       : 8;
    uint32_t weatherStatus : 4;
    uint32_t __pad3        : 4;
    uint32_t seaLevel_bcd  : 16; // BCD: 0.1|1|10|100
} PacketThermoHygroBaro;

// Type 0x06 (10 bytes)
typedef struct
{
    uint32_t __pad0        : 4;
    uint32_t dewUnder      : 1;
    uint32_t __pad1        : 1;
    uint32_t lowBatt       : 1;
    uint32_t __pad2        : 1;
    uint32_t temp_bcd      : 14; // BCD: 0.1|1|10|100
    uint32_t overUnder     : 1;
    uint32_t temp_sign     : 1;  // SIGN
    uint32_t hum_bcd       : 8;  // BCD: 1|10
    uint32_t dewTemp_bcd   : 8;  // BCD: 1|10
    uint32_t adcBaro       : 9;
    uint32_t __pad3        : 3;
    uint32_t weatherStatus : 4;
    uint32_t __pad4        : 4;
    uint32_t seaLevel_bcd  : 20; // BCD: 0.1|1|10|100|1000
} PacketThermoHygroBaroExt;

// Type 0x0E (1 byte)
typedef struct
{
    uint32_t minute_bcd : 7; // BCD: 1|10
    uint32_t lowBatt    : 1;
} PacketMinute;

// Type 0x0F (5 bytes)
typedef struct
{
    uint32_t minute_bcd : 7; // BCD: 1|10
    uint32_t lowBatt    : 1;
    uint32_t hour_bcd   : 8; // BCD: 1|10
    uint32_t day_bcd    : 8; // BCD: 1|10
    uint32_t month_bcd  : 8; // BCD: 1|10
    uint32_t year_bcd   : 8; // BCD: 1|10
} PacketClock;

#endif // _PROTOCOL_H
