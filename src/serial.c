/***
 *
 * serial.c - Serial device part
 * This code is needed to open and close the serial device while storing,
 * changing and restoring the connection parameters.
 *
 * $Id: serial.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>

// Constants
enum
{
    SERIAL_PRIORITY = -20,
    SERIAL_BUFSIZE  = 4096 - sizeof (size_t),
};

// Ring buffer
typedef struct
{
    size_t  num;
    uint8_t data[SERIAL_BUFSIZE];
} RingBuffer;

// Serial handle
typedef struct
{
    uint8_t    *readPtr;
    RingBuffer *ringBuffer;
    struct termios oldtio;
    int fd;
} SerialHandle;

// Prototypes
static void childSignalHandler(int);
static int openDevice(const char *, struct termios *);

// Open serial device
int serialOpen(const char *device)
{
    pid_t pid;
    SerialHandle *serialHandle;
    RingBuffer   *ringBuffer = NULL;
    int zeroFd;

    serialHandle = (SerialHandle *)malloc(sizeof (SerialHandle));
    if (!serialHandle)
    {
        logger(LOG_ERR, "Memory allocation failed");
        goto failed;
    }
  
    serialHandle->fd = openDevice(device, &(serialHandle->oldtio));

    zeroFd = open("/dev/zero", O_RDWR);
    if (zeroFd < 0)
    {
        logger(LOG_ERR, "Couldn't open serial device: %m");
        goto failed;
    }

    ringBuffer =
        (RingBuffer *)mmap(0, sizeof (RingBuffer),
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED, zeroFd, 0);

    close(zeroFd);

    if ((int)ringBuffer < 0)
    {
        logger(LOG_ERR, "mmap failed: %m");
        goto failed;
    }

    serialHandle->ringBuffer = ringBuffer;    
    serialHandle->readPtr    = ringBuffer->data;

    pid = fork();
    if (pid < 0)
    {
        logger(LOG_ERR, "Fork failed: %m");
        goto failed;
    }

    // Child (Writer)
    if (pid == 0)
    {
        uint8_t *writePtr = ringBuffer->data, ch;
        struct sigaction signalAction;

        // Set child signal handler
        memset(&signalAction, 0, sizeof(signalAction));
	signalAction.sa_handler = childSignalHandler;
	sigaction(SIGTERM, &signalAction, NULL);

        // Increase priority
	if (setpriority(PRIO_PROCESS, 0, SERIAL_PRIORITY) < 0)
	    logger(LOG_WARNING, "Child priority couldn't be increased (%m). Aren't you root?");	    

        while (true)
	{
            while (ringBuffer->num == SERIAL_BUFSIZE)
	        usleep(0);
            if (read(serialHandle->fd, &ch, 1) < 0)
	        continue;
            *writePtr = ch;
            ++ringBuffer->num;
            if (writePtr == ringBuffer->data + SERIAL_BUFSIZE - 1)
	        writePtr = ringBuffer->data;
            else
	        ++writePtr;
	    //logger(LOG_DEBUG, "Byte written (Child): 0x%02X", (int)ch);
        }
    }

    // Parent (Reader)
    return (int)serialHandle;

failed:
    free(serialHandle);
    if (ringBuffer)
	munmap(ringBuffer, sizeof (RingBuffer));
    return -1;
}

void serialClose(int fd)
{
    SerialHandle *serialHandle;

    if (fd < 0)
        return;
    
    serialHandle = (SerialHandle *)fd;
    
    tcsetattr(serialHandle->fd, TCSANOW, &(serialHandle->oldtio));
    close(serialHandle->fd);

    kill(0, SIGTERM);
    wait(0);

    munmap(serialHandle->ringBuffer, sizeof (RingBuffer));
    free(serialHandle);	
}

ssize_t serialRead(int fd, void *dest, size_t size)
{
    SerialHandle *serialHandle = (SerialHandle *)fd;
    RingBuffer *ringBuffer = serialHandle->ringBuffer;
    uint8_t *p = (uint8_t *)dest, *end = (uint8_t *)dest + size;

    while (p < end) 
    {
        while (ringBuffer->num <= 0)
            usleep(0);
   
        *p++ = *serialHandle->readPtr;
        --ringBuffer->num;

        //logger(LOG_DEBUG, "Byte read (Parent): 0x%02X", (int)*(p-1));

        if (serialHandle->readPtr == ringBuffer->data + SERIAL_BUFSIZE - 1)
            serialHandle->readPtr = ringBuffer->data;
        else
            ++serialHandle->readPtr;
    }

    return size;
}

static void childSignalHandler(int sig)
{
    logger(LOG_NOTICE, "Child process terminating...");
    _exit(sig);
}	   

static int openDevice(const char *device, struct termios *oldtio)
{
    int fd;
    struct termios tio;
   
    fd = open(device, O_RDONLY | O_NOCTTY);
    if (fd < 0)
    {
        logger(LOG_ERR, "Serial device \"%s\" couldn't be opened: %m", device);
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(fd, oldtio) < 0)
    {
        logger(LOG_ERR, "Terminal options of \"%s\" couldn't be read: %m",
               device);
        exit(EXIT_FAILURE);
    }

    memset(&tio, 0, sizeof (tio));
    /*
     * B9600   : Set bps rate.
     * CRTSCTS : Output hardware flow control
     * CS8     : 8 bit, no parity, 1 stop bit
     * CLOCAL  : Local connection, no modem contol
     * CREAD   : Enable receiving characters
     */
    tio.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL  | CREAD;

    // Ignore bytes with parity errors
    tio.c_iflag = IGNPAR;

    // Blocking read until 1 character arrives (non-canonical)
    tio.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &tio) < 0)
    {
        logger(LOG_ERR, "Terminal options of \"%s\" couldn't be set: %m",
               device);
        exit(EXIT_FAILURE);
    }

    return fd;
}
