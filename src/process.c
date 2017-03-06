/***
 *
 * process.c - Process input data
 * The functions here are related to the input protocol. 
 * process() reads packets from the stream.
 * The different packet types are processed by the handle* functions.
 *
 * $Id: process.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <string.h>
#include <unistd.h>

typedef struct
{
    // Database record
    DBRecord record;

    // Special handler data
    struct tm clockTime;
} ProcessData;

typedef struct
{
    void (*call)(void *, ProcessData *);
    size_t packetLength;
} PacketHandler;

// Conversion functions
static inline uint32_t    bcd2bin(uint32_t, uint32_t);
static inline int32_t     sbcd2bin(uint32_t, uint32_t, uint32_t);
static inline int         channel2Index(uint8_t);
static inline const char *ws2str(uint8_t);

// Checksum
static inline bool checksum(const uint8_t *, size_t, uint8_t);

// Time source handling
static time_t timeClock(ProcessData *);
static time_t timeSystem(ProcessData *);
typedef time_t (*TimeSource)(ProcessData *);

// Different packet handlers
static void handleWind(void *, ProcessData *);
static void handleRain(void *, ProcessData *);
static void handleThermoHygro(void *, ProcessData *);
static void handleMushroom(void *, ProcessData *);
static void handleThermo(void *, ProcessData *);
static void handleThermoHygroBaro(void *, ProcessData *);
static void handleThermoHygroBaroExt(void *, ProcessData *);
static void handleMinute(void *, ProcessData *);
static void handleClock(void *, ProcessData *);

// Packet handler registry
static const PacketHandler packetHandler[] =
{
    /* 0x00 */ { handleWind,               PL_WIND                  },
    /* 0x01 */ { handleRain,               PL_RAIN                  },
    /* 0x02 */ { handleThermoHygro,        PL_THERMO_HYGRO          },
    /* 0x03 */ { handleMushroom,           PL_MUSHROOM              },
    /* 0x04 */ { handleThermo,             PL_THERMO                },
    /* 0x05 */ { handleThermoHygroBaro,    PL_THERMO_HYGRO_BARO     },
    /* 0x06 */ { handleThermoHygroBaroExt, PL_THERMO_HYGRO_BARO_EXT },
    /* 0x07 */ { NULL },
    /* 0x08 */ { NULL },
    /* 0x09 */ { NULL },
    /* 0x0A */ { NULL },
    /* 0x0B */ { NULL },
    /* 0x0C */ { NULL },
    /* 0x0D */ { NULL },
    /* 0x0E */ { handleMinute,             PL_MINUTE                },
    /* 0x0F */ { handleClock,              PL_CLOCK                 },
};

void processPackets(int input, ReadFunction readFunc, bool useClockTime)
{
    ProcessData data;
    int status = 0;
    size_t length = 1;
    uint8_t buffer[PL_MAX], packetType = 0xFF;
    time_t currTime, newTime;
    const PacketHandler *handler = NULL;
    TimeSource timeSource = useClockTime ? timeClock : timeSystem;

    memset(&data, 0, sizeof (data));
    data.record.bthWeatherStatus =
    data.record.extbthWeatherStatus = "undefined";
    currTime = timeSource(&data);

    while (readFunc(input, buffer, length) > 0)
    {
        switch (status)
        {
	case 0: // Process first 0xFF
	case 1: // Process second 0xFF
            if (buffer[0] == 0xFF)
                ++status;
            else
                status = 0;
            break;
             
	case 2: // Process packet type
	    packetType = buffer[0];
            if (packetType >= ARRAY_SIZE(packetHandler) ||
	        !packetHandler[packetType].call)
            {
                logger(LOG_WARNING, "Got packet with unknown type 0x%X", buffer[0]);
                status = 0;
		break;
            }
	    handler = &(packetHandler[packetType]);
            length = handler->packetLength + 1; // packet + checksum
            ++status;        
            break;

        case 3: // Process packet
            if (checksum(buffer, handler->packetLength, packetType))
                handler->call(buffer, &data);
            else
                logger(LOG_WARNING, "Got packet with invalid checksum");
            status = 0, length = 1;
            break;
        }

	newTime = timeSource(&data);
	// Check for valid time (Clock data maybe not available!)
	if (newTime >= 0 && newTime - currTime >= 60)
	{
            data.record.time = currTime = newTime;
	    DBWriteRecord(&data.record);
        }
    }
}

/*
 * Conversion functions
 */

// BCD to binary conversion
static inline uint32_t bcd2bin(uint32_t bcd, uint32_t bits)
{
    uint32_t res = 0, i, n = 1;
    for (i = 0; i < bits; i += 4)
    {
        res += n * ((bcd >> i) & 0xF); 
        n *= 10;
    }
    return res;
}

// Signed BCD to binary conversion
static inline int32_t sbcd2bin(uint32_t sign, uint32_t bcd, uint32_t bits)
{
    return ((sign ? -1 : 1) * (int32_t)bcd2bin(bcd, bits));
}

static inline int channel2Index(uint8_t channel)
{
    switch (channel)
    {
    //case CHANNEL1: return 0;
    case CHANNEL2: return 2;
    case CHANNEL3: return 3;
    }
    return 0; // ???
}

static inline const char *ws2str(uint8_t status)
{
    switch (status)
    {
    case WS_SUNNY:       return "sunny";
    case WS_HALF_CLOUDY: return "half cloudy";
    case WS_CLOUDY:      return "cloudy";
    case WS_RAINY:       return "rainy";
    }
    return "undefined";
}

/*
 * Checksum calculation
 */

static inline bool checksum(const uint8_t *p, size_t length, uint8_t type)
{
    const uint8_t *end = p + length;
    uint32_t sum = 0xFF + 0xFF + type;
    while (p < end)
        sum += *p++;
    return ((sum & 0xFF) == *end);
}

/*
 * Time source handling
 */

static time_t timeClock(ProcessData *data)
{
    if (data->clockTime.month != 0 && data->clockTime.day != 0)
        return mktime(&(data->clockTime));
    return -1;
}

static time_t timeSystem(ProcessData *data)
{
    // Rounded to 1 minute
    time_t t = time(NULL);
    return (t - t % 60);
}

/*
 * Different packet handlers
 */

static void handleWind(void *packet, ProcessData *data)
{
    PacketWind *wind = (PacketWind *)packet;

    if (wind->gustOver || wind->avrgOver || wind->chillOver)
        logger(LOG_WARNING, "A value was over range in packet Wind");

    if (wind->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Wind");

    data->record.windDir  = bcd2bin(wind->dir_bcd, 12);
    
    if (!wind->gustOver)
        data->record.windGust = bcd2bin(wind->gustSpeed_bcd, 12) * 0.1;

    if (!wind->avrgOver)
        data->record.windAvrg = bcd2bin(wind->avrgSpeed_bcd, 12) * 0.1;

    if (!wind->chillNoData && !wind->chillOver)
        data->record.windChill = sbcd2bin(wind->chill_sign, wind->chill_bcd, 8);
}

static void handleRain(void *packet, ProcessData *data)
{
    PacketRain *rain = (PacketRain *)packet;
    struct tm tm;

    if (rain->rateOver || rain->totalOver || rain->yestOver)
        logger(LOG_WARNING, "A value was over range in packet Rain");

    if (rain->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Rain");

    if (!rain->rateOver)
        data->record.rainCurrent = bcd2bin(rain->rate_bcd, 12);
    
    if (!rain->totalOver)
    {
        data->record.rainTotal   = bcd2bin(rain->total_bcd, 20) * 0.1;

        tm.tm_year = bcd2bin(rain->totalYear_bcd, 8) + 100;
        tm.tm_mon  = bcd2bin(rain->totalMonth_bcd, 8) - 1;
        tm.tm_mday = bcd2bin(rain->totalDay_bcd, 8);
        tm.tm_hour = bcd2bin(rain->totalHour_bcd, 8);
        tm.tm_min  = bcd2bin(rain->totalMinute_bcd, 8);
        data->record.rainTotalStart = mktime(&tm);
    }

    if (!rain->yestOver)
        data->record.rainYesterday = bcd2bin(rain->yest_bcd, 16);
}

static void handleThermoHygro(void *packet, ProcessData *data) 
{
    PacketThermoHygro *thermoHygro = (PacketThermoHygro *)packet;
    int i = 0;

    if (thermoHygro->dewUnder || thermoHygro->overUnder)
        logger(LOG_WARNING, "A value was over range in packet Thermo+Hygro");

    if (thermoHygro->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Thermo+Hygro");

    i = channel2Index(thermoHygro->channel);
    
    if (!thermoHygro->overUnder)
        data->record.channelTemp[i] = sbcd2bin(thermoHygro->temp_sign, thermoHygro->temp_bcd, 14) * 0.1;
    
    data->record.channelHumidity[i] = bcd2bin(thermoHygro->hum_bcd, 8);
    
    if (!thermoHygro->dewUnder)
        data->record.channelDewTemp[i]  = bcd2bin(thermoHygro->dewTemp_bcd, 8);
}

static void handleMushroom(void *packet, ProcessData *data)
{
    PacketMushroom *mushroom = (PacketMushroom *)packet;

    if (mushroom->dewUnder || mushroom->overUnder)
        logger(LOG_WARNING, "A value was over range in packet Mushroom");

    if (mushroom->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Mushroom");

    if (!mushroom->overUnder)
        data->record.mushroomTemp     = sbcd2bin(mushroom->temp_sign, mushroom->temp_bcd, 14) * 0.1;
    
    data->record.mushroomHumidity = bcd2bin(mushroom->hum_bcd, 8);
    
    if (!mushroom->dewUnder)
        data->record.mushroomDewTemp  = bcd2bin(mushroom->dewTemp_bcd, 8);
}

static void handleThermo(void *packet, ProcessData *data)
{
    PacketThermo *thermo = (PacketThermo *)packet;

    if (thermo->overUnder)
        logger(LOG_WARNING, "A value was over range in packet Thermo");

    if (thermo->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Thermo");

    if (!thermo->overUnder)
        data->record.channelTemp[channel2Index(thermo->channel)] =
        sbcd2bin(thermo->temp_sign, thermo->temp_bcd, 14) * 0.1;
}

static void handleThermoHygroBaro(void *packet, ProcessData *data)
{
    PacketThermoHygroBaro *thermoHygroBaro = (PacketThermoHygroBaro *)packet;

    if (thermoHygroBaro->dewUnder || thermoHygroBaro->overUnder)
        logger(LOG_WARNING, "A value was over range in packet Thermo+Hygro+Baro");

    if (thermoHygroBaro->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Thermo+Hygro+Baro");

    if (!thermoHygroBaro->overUnder)
        data->record.bthTemp = sbcd2bin(thermoHygroBaro->temp_sign, thermoHygroBaro->temp_bcd, 14) * 0.1;
    
    data->record.bthHumidity = bcd2bin(thermoHygroBaro->hum_bcd, 8);
    
    if (!thermoHygroBaro->dewUnder)
        data->record.bthDewTemp = bcd2bin(thermoHygroBaro->dewTemp_bcd, 8);
    
    data->record.bthBaro     = thermoHygroBaro->adcBaro + 795;
    strcpy(data->record.bthWeatherStatus, ws2str(thermoHygroBaro->weatherStatus));
}

static void handleThermoHygroBaroExt(void *packet, ProcessData *data)
{
    PacketThermoHygroBaroExt *thermoHygroBaroExt = (PacketThermoHygroBaroExt *)packet;

    if (thermoHygroBaroExt->dewUnder || thermoHygroBaroExt->overUnder)
        logger(LOG_WARNING, "A value was over range in packet Thermo+Hygro+Baro(External)");

    if (thermoHygroBaroExt->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Thermo+Hygro+Baro(External)");

    if (!thermoHygroBaroExt->overUnder)
        data->record.extbthTemp = sbcd2bin(thermoHygroBaroExt->temp_sign, thermoHygroBaroExt->temp_bcd, 14) * 0.1;
    
    data->record.extbthHumidity = bcd2bin(thermoHygroBaroExt->hum_bcd, 8);
    
    if (!thermoHygroBaroExt->dewUnder)
        data->record.extbthDewTemp = bcd2bin(thermoHygroBaroExt->dewTemp_bcd, 8);
    
    data->record.extbthBaro = thermoHygroBaroExt->adcBaro + 600;
    strcpy(data->record.extbthWeatherStatus, ws2str(thermoHygroBaroExt->weatherStatus));
}

static void handleMinute(void *packet, ProcessData *data)
{
    PacketMinute *minute = (PacketMinute *)packet;

    if (minute->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Minute");

    data->clockTime.tm_min = bcd2bin(minute->minute_bcd, 7);
}

static void handleClock(void *packet, ProcessData *data)
{
    PacketClock *clock = (PacketClock *)packet;
    if (clock->lowBatt)
        logger(LOG_WARNING, "Got a battery low signal in packet Clock");

    data->clockTime.tm_year = bcd2bin(clock->year_bcd, 8) + 100;
    data->clockTime.tm_mon  = bcd2bin(clock->month_bcd, 8) - 1;
    data->clockTime.tm_mday = bcd2bin(clock->day_bcd, 8);
    data->clockTime.tm_hour = bcd2bin(clock->hour_bcd, 8);
    data->clockTime.tm_min  = bcd2bin(clock->minute_bcd, 7);
}
