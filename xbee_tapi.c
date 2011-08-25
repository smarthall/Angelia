#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <asm-generic/ioctls.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

// File Descriptor of the USB device
int usb_fd = 0;

// Serial Handling
static int (*next_cfsetispeed)(struct termios *termios_p, speed_t speed) = NULL;
static int (*next_cfsetospeed)(struct termios *termios_p, speed_t speed) = NULL;
static int (*next_tcgetattr)(int fd, struct termios *termios_p) = NULL;
static int (*next_tcsetattr)(int fd, int optional_actions, const struct termios *termios_p) = NULL;

// Lower level IOCTLS
static int (*next_ioctl)(int fd, int request, void *data) = NULL;
static int (*next_open)(const char *pathname, int flags, mode_t mode) = NULL;




int cfsetospeed(struct termios *termios_p, speed_t speed) {
     if (next_cfsetospeed == NULL) next_cfsetospeed = dlsym(RTLD_NEXT, "cfsetospeed");

     printf("cfsetospeed\n");

     return next_cfsetospeed(termios_p, speed);
}

int cfsetispeed(struct termios *termios_p, speed_t speed) {
     if (next_cfsetispeed == NULL) next_cfsetispeed = dlsym(RTLD_NEXT, "cfsetispeed");

     printf("cfsetispeed\n");

     return next_cfsetispeed(termios_p, speed);
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    if (next_tcsetattr == NULL) next_tcsetattr = dlsym(RTLD_NEXT, "tcsetattr");

    printf("tcsetattr\n");

    return next_tcsetattr(fd, optional_actions, termios_p);
}

int tcgetattr(int fd, struct termios *termios_p) {
    if (next_tcgetattr == NULL) next_tcgetattr = dlsym(RTLD_NEXT, "tcgetattr");

    printf("tcgetattr\n");

    return next_tcgetattr(fd, termios_p);
}

int open(const char *pathname, int flags, mode_t mode) {
    int response;
    const char *usbpath = "/dev/ttyACM0";

    if (next_open == NULL) next_open = dlsym(RTLD_NEXT, "open");

    response = next_open(pathname, flags, mode);

    if (response != -1 && strcmp(usbpath, pathname) == 0) {
        usb_fd = response;
        printf("Opened USB @ fd=%d\n", response);
    }

    return response;
}

int ioctl(int fd, int request, void *data)
{
  if (next_ioctl == NULL) next_ioctl = dlsym(RTLD_NEXT, "ioctl");

  if (usb_fd != 0 && fd == usb_fd) {
      printf("IOCTL (0x%04X) ", request);
      if (request == TIOCMGET) {
        printf("TIOCMGET");
      } else if (request == TIOCMSET) {
        printf("TIOCMSET");
      }
      printf("\n");
  }

  return next_ioctl(fd, request, data);
}

