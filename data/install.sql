/***
 *
 * install.sql
 *
 * $Id: install.sql,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

/* Create data table */
CREATE TABLE data 
(
    /* Primary key */
    time                 INT UNSIGNED PRIMARY KEY,
    /* Wind data*/
    windDir              SMALLINT UNSIGNED,
    windGust             FLOAT,
    windAvrg             FLOAT,
    windChill            TINYINT,
    /* Rain data */ 
    rainCurrent          SMALLINT UNSIGNED,
    rainTotal            FLOAT,
    rainTotalStart       INT UNSIGNED,
    rainYesterday        SMALLINT UNSIGNED,
    /* Channel 1 data */ 
    channel1Temp         FLOAT,
    channel1Humidity     TINYINT UNSIGNED,
    channel1DewTemp      TINYINT UNSIGNED,
    /* Channel 2 data */
    channel2Temp         FLOAT,
    channel2Humidity     TINYINT UNSIGNED,
    channel2DewTemp      TINYINT UNSIGNED,
    /* Channel 3 data */   
    channel3Temp         FLOAT,
    channel3Humidity     TINYINT UNSIGNED,
    channel3DewTemp      TINYINT UNSIGNED,
    /* Mushroom data */
    mushroomTemp         FLOAT,
    mushroomHumidity     TINYINT UNSIGNED,
    mushroomDewTemp      TINYINT UNSIGNED,
    /* BTH data */
    bthTemp              FLOAT,
    bthHumidity          TINYINT UNSIGNED,
    bthDewTemp           TINYINT UNSIGNED,
    bthBaro              TINYINT UNSIGNED,
    bthWeatherStatus     VARCHAR(16),
    /* EXTBTH data */
    extbthTemp           FLOAT,
    extbthHumidity       TINYINT UNSIGNED,
    extbthDewTemp        TINYINT UNSIGNED,
    extbthBaro           SMALLINT UNSIGNED,
    extbthWeatherStatus  VARCHAR(16)
);
