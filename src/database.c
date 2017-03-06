/***
 *
 * database.c - Database part
 * Here you can find all functions that deal with the database
 *
 * $Id: database.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <dbi/dbi.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Database data
static char      table[256];
static dbi_conn  conn;

// Query string
static const char *queryString =
"INSERT INTO %s VALUES ("
// Primary key 
"%u,"
// Wind data
"%u,%.1f,%.1f,%d,"
// Rain data
"%u,%.1f,%u,%u"
// Channel data
"%.1f,%u,%u,"
"%.1f,%u,%u,"
"%.1f,%u,%u,"
// Mushroom data
"%.1f,%u,%u,"
// BTH data
"%.1f,%u,%u,%u,'%s',"
// EXTBTH data
"%.1f,%u,%u,%u,'%s')\n";

static const char *time2str(time_t t)
{
    char *p = ctime(&t);
    *(p + strlen(p) - 1) = '\0';
    return p;
}

// Initialize dbi interface and connect to database
void DBOpen(const char *driverPath, const char *driverType,
            const char *dbHost, int dbPort,
            const char *dbUser, const char *dbPassword,
            const char *dbName, const char *dbTable)
{
    if (dbi_initialize(driverPath) < 1)
    {
        logger(LOG_ERR, "No dbi drivers found");
        exit(EXIT_FAILURE);
    }

    conn = dbi_conn_new(driverType);
    if (conn == NULL)
    {
        logger(LOG_ERR, "Couldn't create dbi connection, wrong database type?");
        exit(EXIT_FAILURE);
    }

    dbi_conn_set_option(conn, "host", dbHost);
    dbi_conn_set_option_numeric(conn, "port", dbPort);
    dbi_conn_set_option(conn, "username", dbUser);
    dbi_conn_set_option(conn, "password", dbPassword);
    dbi_conn_set_option(conn, "dbname", dbName);

    if (dbi_conn_connect(conn) < 0)
    {
        const char *error;
        dbi_conn_error(conn, &error);
        logger(LOG_ERR, error);
        exit(EXIT_FAILURE);
    }

    strncpy(table, dbTable, sizeof (table));
}

void DBClose()
{
    dbi_conn_close(conn);
    dbi_shutdown();
}
 
// Read internal DBRecord and send query to create external record
void DBWriteRecord(DBRecord *record)
{
    static int count = 0;
    
    dbi_result result =
    dbi_conn_queryf(conn, queryString, table,
		    // Primary key
		    record->time,
		    // Wind data
		    record->windDir,
		    record->windGust,
		    record->windAvrg,
		    record->windChill,
		    // Rain data
		    record->rainCurrent,
		    record->rainTotal,
	            record->rainTotalStart,
		    record->rainYesterday,
		    // Channel 1 data
		    record->channelTemp[0],
		    record->channelHumidity[0],
		    record->channelDewTemp[0],
		    // Channel 2 data
		    record->channelTemp[1],
		    record->channelHumidity[1],
		    record->channelDewTemp[1],
		    // Channel 3 data
		    record->channelTemp[2],
		    record->channelHumidity[2],
		    record->channelDewTemp[2],
		    // Mushroom data
		    record->mushroomTemp,
		    record->mushroomHumidity,
		    record->mushroomDewTemp,
		    // BTH data
		    record->bthTemp,
		    record->bthHumidity,
		    record->bthDewTemp,
		    record->bthBaro,
		    record->bthWeatherStatus,
		    // EXTBTH data
		    record->extbthTemp,
		    record->extbthHumidity,
		    record->extbthDewTemp,
		    record->extbthBaro,
		    record->extbthWeatherStatus);
		    
    if (!result)
    {
        const char *error;
        dbi_conn_error(conn, &error);
        if (strstr(error, "Duplicate entry"))
	    logger(LOG_WARNING, 
                   "Duplicate database entry: \"%s\"", time2str(record->time));
        else
            logger(LOG_ERR, error);
    }
    else
    {
	++count;
        logger(LOG_INFO, "Database entry %d: \"%s\"", count, time2str(record->time));    
        if (count % 100 == 0)
            logger(LOG_NOTICE, "%d database records written", count);
    }
}
