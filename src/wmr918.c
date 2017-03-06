/***
 *
 * Program name: wmr918
 * Title:        wmr918 middleware daemon
 * Author:       Andre Colomb, Daniel Mendler
 * Co-Authors:   Armin Hanle, Konrad Sailer, Linda Kagelmacher, Peter Stock
 * 
 * Description:
 * wmr918 reads data from a weather station (model WMR918), either from
 * a serial device or from a file with the raw data stream. It processes the
 * data and extracts data packets.
 * 
 * Additionally, it checks for any error messages from the weather station.
 * Every full minute, the information got from the data stream
 * is stored in an SQL database table. The database is accessed
 * through the libdbi interface supporting different SQL database types.
 *
 * The program's configuration is read from a file.
 * The database and the DBI driver can be configured.
 *
 * $Id: wmr918.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// getopt-Options
static const struct option longOptions[] =
{
    { "config",       1, NULL, 'c' },
    { "nodetach",     0, NULL, 'n' },
    { "wsclock",      0, NULL, 'w' },
    { "verbose",      0, NULL, 'v' },
    { "help",         0, NULL, '?' },
    { "version",      0, NULL, 'V' },
    { NULL,           0, NULL, 0   },
};

// Help string
static const char *help =
PACKAGE " Version " VERSION "\n"
"Usage: %s [Options] [Stream files...]\n"
"Options:\n"
"  -c, --config <file> Read configuration from file\n"
"  -n, --nodetach      Don't detach from controlling terminal\n"
"  -w, --wsclock       Use weather station clock instead of system time\n"
"  -v, --verbose       Be more verbose\n"
"  -?, --help          Print this page and exit\n"
"      --version       Print version information and exit\n";

// Version string
static const char *version =
PACKAGE " Version " VERSION "\n";

// Prototypes
static void signalHandler(int);
static void cleanup();

// Variables used by cleanup
static int input = -1;
static bool readFromDevice = true;

// Configuration
static Cfg cfg[] =
{
    // Database configuration
    { "database.name"     },
    { "database.host"     },
    { "database.port"     },
    { "database.user"     },
    { "database.password" },
    { "database.table"    },
    // Driver configuration
    { "driver.path"       },
    { "driver.type"       },
    // Device configuration
    { "device.name"       },
    { NULL                },
};

int main(int argc, char *argv[])
{
    bool detach = true, useClockTime = false;
    char configFile[256] = CONFIG_FILE;
    struct sigaction signalAction;  

    // Parse command line
    while (true)
    {
        int option = getopt_long(argc, argv, "+c:nwv?", longOptions, NULL);
        if (option == EOF)
            break;
        switch(option)
        {
        case 'c': // Config file
            strncpy(configFile, optarg, sizeof (configFile));
            break;

        case 'n': // Nodetach from controlling terminal
            detach = false;
            break;

        case 'w': // Use weather station clock instead of system time
	    useClockTime = true;
	    break;

        case 'v': // Be more verbose
            loggerMoreVerbose();
            break;

        case '?': // Help
            printf(help, argv[0]);
            exit(EXIT_SUCCESS);

        case 'V': // Version
            printf(version);
            exit(EXIT_SUCCESS);
            break;

        default: // Unknown option
            printf(help, argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    // Read from device?
    readFromDevice = (optind >= argc);
    
    if (readFromDevice)
    {
        // Is this a daemon?
        if (detach)
        {
            logger(LOG_NOTICE, "Detaching from controlling terminal...");
            if (daemon(1, 0) < 0)
            {
                logger(LOG_ERR, "Couldn't detach from controlling terminal: %m");
                exit(EXIT_FAILURE);
            }
        }

        // Lock pid file if daemon reads from device
        // because we want only one daemon running.
        lockPidFile(PID_FILE);
    }

    // Install exit and signal handler
    atexit(cleanup);
    memset(&signalAction, 0, sizeof (signalAction));
    signalAction.sa_handler = signalHandler;
    sigaction(SIGTERM, &signalAction, NULL);
    sigaction(SIGINT, &signalAction, NULL);

    // Read configuration
    logger(LOG_NOTICE, "Reading configuration...");
    cfgRead(cfg, configFile);

    // Connect to database
    logger(LOG_NOTICE, "Connecting to database %s@%s...",
           cfgValue(cfg, "database.name"), cfgValue(cfg, "database.host"));
    DBOpen(cfgValue(cfg, "driver.path"),   cfgValue(cfg, "driver.type"),
           cfgValue(cfg, "database.host"), atoi(cfgValue(cfg, "database.port")),
           cfgValue(cfg, "database.user"), cfgValue(cfg, "database.password"),
           cfgValue(cfg, "database.name"), cfgValue(cfg, "database.table"));

    // Read from device?
    if (readFromDevice)
    {
        logger(LOG_NOTICE, "Opening serial device \"%s\"...", cfgValue(cfg, "device.name"));
        input = serialOpen(cfgValue(cfg, "device.name"), &oldtio);

        logger(LOG_NOTICE, "Starting processing packets from device \"%s\"...",
               cfgValue(cfg, "device.name"));

        processPackets(input, serialRead, useClockTime);
    }
    // Read from files.
    else
    {
        do
        {
            logger(LOG_NOTICE, "Opening stream file \"%s\"...", argv[optind]);
            input = open(argv[optind], O_RDONLY);
            if (input < 0)
            {
                logger(LOG_WARNING, "Stream file \"%s\"coudn't be opened: %m",
                       argv[optind]);
	        continue;
            }

            logger(LOG_NOTICE, "Starting processing packets from file \"%s\"...",
                   argv[optind]);
         
            processPackets(input, read, true);
            logger(LOG_NOTICE, "Processing packets from file \"%s\" finished",
                   argv[optind]);
       
            close(input);
        }
	while (++optind < argc);
    }
    
    exit(EXIT_SUCCESS);
}

// SIGINT and SIGTERM handler
static void signalHandler(int sig)
{
    logger(LOG_NOTICE, "%s signal received",
           sig == SIGTERM ? "TERM" : "INT");
    exit(sig);
}

// atexit handler
static void cleanup()
{ 
    logger(LOG_NOTICE, "Disconnecting from database...");
    DBClose();

    if (readFromDevice)
        serialClose(input);
    else
        // Close current file
        close(input);

    logger(LOG_NOTICE, "Terminating...");
}
