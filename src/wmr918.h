/***
 *
 * wmr918.h - All functions
 * Changes to this file need a recompilation.
 *
 * $Id: wmr918.h,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#ifndef _WMR918_H
#define _WMR918_H

/*****************************************
 * Includes
 */

// Configuration file
#include <config.h>

// Own headers
#include "dbrecord.h"
#include "protocol.h"

// Standard headers
#include <stdint.h>
#include <syslog.h>

/*****************************************
 * Type definitions & Macros
 */

typedef enum { false, true } bool;

#define ARRAY_SIZE(array) (sizeof (array) / sizeof ((array)[0]))

typedef ssize_t (*ReadFunction)(int, void *, size_t);


/*****************************************
 * Logger functions (logger.c)
 */

/*
 * Logger Levels:
 *     -> LOG_ERR:     Fatal error with program termination
 *     -> LOG_WARNING: Error
 *     -> LOG_NOTICE:  Information
 *     -> LOG_INFO:    Unimportant information
 */
void loggerMoreVerbose();
void logger(int, const char *, ...); // "%m" == errno string

/*****************************************
 * Serial driver functions (serial.c)
 */

int serialOpen(const char *);
void serialClose(int);
ssize_t serialRead(int, void *, size_t);

/*****************************************
 * Packet processing function (process.c)
 */

void processPackets(int, ReadFunction, bool);

/*****************************************
 * Database functions (database.c)
 */

void DBOpen(const char *, const char *, 
            const char *, int,
            const char *, const char *,
            const char *, const char *);
void DBClose();
void DBWriteRecord(DBRecord *);

/*****************************************
 * Configuration reader functions (cfg.c)
 */

enum
{
    CFG_VALUE_SIZE = 256,
    CFG_LINE_SIZE  = 512,
};

typedef struct
{
    char *name;
    char  value[CFG_VALUE_SIZE];
    bool  valid;
} Cfg;

void cfgRead(Cfg *, const char *);
const char *cfgValue(Cfg *, const char *);

/*****************************************
 * Miscellaneous functions (misc.c)
 */

void lockPidFile(const char *);
char *strtrim(char *);

#endif // _WMR918_H
