#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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
#include "xbee.h"

// File Descriptor of the USB device
static int usb_fd = 0, last_termios_set = 0, xbee_is_init = 0;
static uint8_t local_addr[8];
static struct termios last_termios;

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

void __attribute__ ((constructor)) xbee_tapi_init(void) {
    next_select = dlsym(RTLD_NEXT, "select");
    next_read = dlsym(RTLD_NEXT, "read");
    next_write = dlsym(RTLD_NEXT, "write");
    next_tcsetattr = dlsym(RTLD_NEXT, "tcsetattr");
    next_tcgetattr = dlsym(RTLD_NEXT, "tcgetattr");
    next_open = dlsym(RTLD_NEXT, "open");
    next_close = dlsym(RTLD_NEXT, "close");
    next_ioctl = dlsym(RTLD_NEXT, "ioctl");
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    return next_select(nfds, readfds, writefds, exceptfds, timeout);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
     /*if (fildes == usb_fd) {
         printf("<");
         fflush(stdout);
     }*/

     return next_read(fildes, buf, nbyte);
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
    uint8_t *packet, i;
    uint8_t buffer[1024];
    uint8_t coordinator[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t remote[8] = {0x00, 0x13, 0xa2, 0x00, 0x40, 0x76, 0x35, 0x22};
    int resp;

    if (fildes == usb_fd) {
        if (xbee_is_init == 0) {
            // TODO Confirm the right firmware on local XBee (API mode)
            packet = xbee_at_packet("VR");
            memset(buffer, 0, 1024);
            while (xbee_frame_type(buffer) != XBEE_CMD_ATR) {
                print_xbee_packet("Sent", packet);
                resp = next_write(fildes, packet, xbee_packet_size(packet));
                xbee_read(buffer, 1024);
                if (valid_xbee_packet(buffer)) print_xbee_packet("Recv", buffer);
            }
            free_xbee_packet(packet);

            // TODO Confirm the right firmware on remote XBee (AT mode)
            //packet = xbee_rat_packet("VR", remote);
            //print_xbee_packet("Sent", packet);
            //resp = next_write(fildes, packet, xbee_packet_size(packet));
            //free_xbee_packet(packet);

            // TODO Get the local XBee address
            packet = xbee_at_packet("SH");
            memset(buffer, 0, 1024);
            while (xbee_frame_type(buffer) != XBEE_CMD_ATR) {
                print_xbee_packet("Sent", packet);
                resp = next_write(fildes, packet, xbee_packet_size(packet));
                xbee_read(buffer, 1024);
                if (valid_xbee_packet(buffer)) print_xbee_packet("Recv", buffer);
            }
            free_xbee_packet(packet);

            memcpy(local_addr, buffer + 8, 4);

            packet = xbee_at_packet("SL");
            memset(buffer, 0, 1024);
            while (xbee_frame_type(buffer) != XBEE_CMD_ATR) {
                print_xbee_packet("Sent", packet);
                resp = next_write(fildes, packet, xbee_packet_size(packet));
                xbee_read(buffer, 1024);
                if (valid_xbee_packet(buffer)) print_xbee_packet("Recv", buffer);
            }
            free_xbee_packet(packet);

            memcpy(local_addr + 4, buffer + 8, 4);

            printf("Our Address: ");
            for (i=0; i < 8; i++) {
                printf("%02x", local_addr[i]);
            }
            printf("\n");


            // TODO Set the remote destination to our XBee address
            //packet = xbee_rat_packet_param("DH", remote, 4, localaddress_h);
            //resp = next_write(fildes, packet, xbee_packet_size(packet));
            //free_xbee_packet(packet);
            //packet = xbee_rat_packet_param("DL", remote, 4, localaddress_l);
            //resp = next_write(fildes, packet, xbee_packet_size(packet));
            //free_xbee_packet(packet);

            // We've initialized the XBee
            xbee_is_init = 1;
        }

        // Send the packet
        packet = xbee_tx_packet(remote, 0x00, nbyte, buf);
        resp = next_write(fildes, packet, xbee_packet_size(packet));
        free_xbee_packet(packet);

        return nbyte;
    }

    return next_write(fildes, buf, nbyte);
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    struct termios local_term = *termios_p;

    if (fd == usb_fd) {
        // Store the settings for the close
        last_termios = *termios_p;
        last_termios_set = 1;

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

    if ((fd == usb_fd) && (last_termios_set == 1)) {
        termios_p = &last_termios;
        debug_termios("faked", termios_p);
        response = 0;
    } else if (fd == usb_fd) {
        response = next_tcgetattr(fd, termios_p);
        last_termios = *termios_p;
        last_termios_set = 1;
        debug_termios("real", termios_p);
    } else {
        response = next_tcgetattr(fd, termios_p);
    }

    return response;
}

int open(const char *pathname, int flags, mode_t mode) {
    int response;
    const char *usbpath = "/dev/ttyUSB0";

    // TODO Gather configuration from environment

    response = next_open(pathname, flags, mode);

    // If we opened the USB device successfully
    if (response != -1 && strcmp(usbpath, pathname) == 0) {
        // Record the file descriptor
        usb_fd = response;

        //Debug message
        printf("Opened USB @ fd=%d\n", response);
    }

    return response;
}

int close(int fd) {
    // If the USB device is getting closed
    if (fd == usb_fd) {
        // Forget the file descriptor
        usb_fd = 0;
        last_termios_set = 0;

        // Restore settings
        next_tcsetattr(fd, TCSANOW | TCSADRAIN, &last_termios);
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
      return 0;
  }

  return next_ioctl(fd, request, data);
}

int xbee_read(uint8_t *buf, size_t buflen) {
    struct timeval to1, to2;
    fd_set read_fds;
    int fd_count;
    size_t length = 0, readlen;
    uint8_t *p;

    to1.tv_sec = 0;
    to1.tv_usec = 80000;

    p = buf;

    while (length < buflen) {
        // Get setup
        FD_ZERO(&read_fds);
        FD_SET(usb_fd, &read_fds);

        // Wait for data
        fd_count = next_select(usb_fd + 1, &read_fds, NULL, NULL, &to1);
        if (fd_count == 0) return length; // Timeout
        if (fd_count == -1) return -1; // Error

        // Read the data
        readlen = next_read(usb_fd, p, (buflen - length > 1024) ? 1024 : buflen - length);
        if (readlen < 0) return -1; // Read Error

        // Move pointers, update length
        p += readlen;
        length += readlen;
    }

    return length;
}


