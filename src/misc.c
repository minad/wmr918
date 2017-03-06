/***
 *
 * misc.c - Miscellaneous functions
 *
 * $Id: misc.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Lock pid file
void lockPidFile(const char *fileName)
{
    struct flock lock;
    char buffer[16];
    int fd;

    // Open pid file
    fd = open(fileName, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
    {
        logger(LOG_ERR, "Couldn't open pid file: %m");
        exit(EXIT_FAILURE);
    }
 
    memset(&lock, 0, sizeof (lock));
    lock.l_type   = F_WRLCK;
    lock.l_whence = SEEK_SET;

    // Lock file
    if (fcntl(fd, F_SETLK, &lock) < 0)   
    {
        // Already locked?
        if (errno == EAGAIN || errno == EACCES)
        {
            read(fd, buffer, sizeof (buffer));
            logger(LOG_ERR, "Daemon already running (Pid = %d)",
                   atoi(buffer));
            exit(EXIT_SUCCESS);
        }
        logger(LOG_ERR, "Couldn't lock pid file: %m");
        exit(EXIT_FAILURE);
    }
 
    // Clean file
    if (ftruncate(fd, 0) < 0)
    {
        logger(LOG_ERR, "Couldn't clean pid file: %m");
        exit(EXIT_FAILURE);
    }

    // Write own pid
    snprintf(buffer, sizeof (buffer), "%d", getpid());
    if (write(fd, buffer, strlen(buffer)) < 0)
    {
        logger(LOG_ERR, "Couldn't write to pid file: %m");
        exit(EXIT_FAILURE);
    }
}

// Remove heading and trailing spaces 
char *strtrim(char *str)
{
    char *p;

    // Trim right blanks
    p = str + strlen(str) - 1;
    while (p >= str && isspace(*p))
        *p-- = '\0';

    // Trim left blanks
    p = str;
    while (*p != '\0' && isspace(*p))
        ++p;
    strcpy(str, p);

    return str;
}
