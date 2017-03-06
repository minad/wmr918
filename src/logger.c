/***
 *
 * logger.c - Logging part
 * Here you can find all functions that deal with logging
 *
 * $Id: logger.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <stdarg.h>

static int minLevel = LOG_ERR;

void loggerMoreVerbose()
{
    if (minLevel < LOG_DEBUG)
        ++minLevel;
}

void logger(int level, const char *format, ...)
{
    static bool initDone = false;
    va_list ap;
    
    if (level > minLevel)
        return;

    // Open syslog if not already done
    if (!initDone)
    {
        // Write also to STDERR
        openlog(PACKAGE, LOG_PERROR | LOG_PID, LOG_DAEMON);
        initDone = true;
    }
    
    va_start(ap, format);
    vsyslog(level, format, ap);
    va_end(ap);
}
