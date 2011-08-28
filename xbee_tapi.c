#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#define _SYS_IOCTL_H
#include <asm-generic/ioctls.h>
#include <bits/ioctl-types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "serial.h"

// File Descriptor of the USB device
int usb_fd = 0;
struct termios last_termios;

// Serial Handling
static int (*next_tcgetattr)(int fd, struct termios *termios_p) = NULL;
static int (*next_tcsetattr)(int fd, int optional_actions, const struct termios *termios_p) = NULL;

// Reading and Writing
static int (*next_open)(const char *pathname, int flags, mode_t mode) = NULL;
static int (*next_close)(int fd) = NULL;
static ssize_t (*next_read)(int fildes, void *buf, size_t nbyte) = NULL;
static ssize_t (*next_write)(int fildes, const void *buf, size_t nbyte) = NULL;
static int (*next_select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) = NULL;

// Lower level IOCTLS
static int (*next_ioctl)(int fd, unsigned long int request, void *data) = NULL;

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    if (next_select == NULL) next_select = dlsym(RTLD_NEXT, "select");

    return next_select(nfds, readfds, writefds, exceptfds, timeout);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
     if (next_read == NULL) next_read = dlsym(RTLD_NEXT, "read");

     /*if (fildes == usb_fd) {
         printf("<");
         fflush(stdout);
     }*/

     return next_read(fildes, buf, nbyte);
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
     if (next_write == NULL) next_write = dlsym(RTLD_NEXT, "write");

     /*if (fildes == usb_fd) {
         printf(">");
         fflush(stdin);
     }*/

     return next_write(fildes, buf, nbyte);
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    struct termios local_term = *termios_p;

    if (next_tcsetattr == NULL) next_tcsetattr = dlsym(RTLD_NEXT, "tcsetattr");

    if (fd == usb_fd) {
        // Store the settings for the close
        last_termios = *termios_p;

        // Change the settings to match the local XBee
        local_term.c_iflag = IGNBRK;
        local_term.c_oflag = 0;
        local_term.c_lflag = 0;
        local_term.c_cflag = (CS8 | CREAD | CLOCAL);
        local_term.c_cc[VMIN]  = 1;
        local_term.c_cc[VTIME] = 0;
        cfsetospeed(&local_term, B9600);
        cfsetispeed(&local_term, B9600);

        // Some debug messages
        debug_termios("requested", termios_p);
        debug_termios("actual", &local_term);
    }

    return next_tcsetattr(fd, optional_actions, &local_term);
}

int tcgetattr(int fd, struct termios *termios_p) {
    int ibaud, obaud, parity, stop;
    int response;

    if (next_tcgetattr == NULL) next_tcgetattr = dlsym(RTLD_NEXT, "tcgetattr");

    response = next_tcgetattr(fd, termios_p);

    if (fd == usb_fd) {
        debug_termios("query", termios_p);
    }

    return response;
}

int open(const char *pathname, int flags, mode_t mode) {
    int response;
    const char *usbpath = "/dev/ttyACM0";

    if (next_open == NULL) next_open = dlsym(RTLD_NEXT, "open");

    // TODO Gather configuration from environment

    response = next_open(pathname, flags, mode);

    // If we opened the USB device successfully
    if (response != -1 && strcmp(usbpath, pathname) == 0) {
        // Record the file descriptor
        usb_fd = response;

        // TODO Establish communication with the XBee
        // TODO Confirm the right firmware on local XBee (API mode)
        // TODO Confirm the right firmware on remote XBee (AT mode)
        // TODO Get the local XBee address
        // TODO Set the remote destination to our XBee address

        //Debug message
        printf("Opened USB @ fd=%d\n", response);
    }

    return response;
}

int close(int fd) {
    if (next_close == NULL) next_close = dlsym(RTLD_NEXT, "close");

    // If the USB device is getting closed
    if (fd == usb_fd) {
        // Forget the file descriptor
        usb_fd = 0;

        // Restore settings
        next_tcsetattr(fd, TCSANOW, &last_termios);
        debug_termios("restored", &last_termios);

        // TODO Set the remote XBee destination back to the original value
        // TODO Return the local XBee to previous settings

        //Debug message
        printf("Closed USB @ fd=%d\n", fd);
    }

    return next_close(fd);
}

int ioctl(int fd, unsigned long int request, void *data)
{
  unsigned int *flags = (unsigned int *)data;
  if (next_ioctl == NULL) next_ioctl = dlsym(RTLD_NEXT, "ioctl");

  if (usb_fd != 0 && fd == usb_fd) {
      printf("IOCTL (0x%04X) ", request);
      if (request == TIOCMGET) {
        printf("TIOCMGET ");
      } else if (request == TIOCMSET) {
        printf("TIOCMSET ");
        if (CHECK_FLAG(*flags, TIOCM_DTR)) printf("DTR ");
        if (CHECK_FLAG(*flags, TIOCM_RTS)) printf("RTS ");
      }
      printf("\n");
  }

  return next_ioctl(fd, request, data);
}

