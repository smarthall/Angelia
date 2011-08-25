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

static int (*next_ioctl)(int fd, int request, void *data) = NULL;
static int (*next_open)(const char *pathname, int flags, mode_t mode) = NULL;
int usb_fd = 0;

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
      if (request == TCGETS) {
        printf("TCGETS");
      } else if (request == TCSETS) {
        printf("TCSETS");
      }
      printf("\n");
  }

  return next_ioctl(fd, request, data);
}

