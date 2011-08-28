#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "serial.h"

struct baud_mapping {
    long baud;
    speed_t speed;
};

static struct baud_mapping baud_lookup_table [] = {
  { 110,    B110 },
  { 1200,   B1200 },
  { 2400,   B2400 },
  { 4800,   B4800 },
  { 9600,   B9600 },
  { 19200,  B19200 },
  { 38400,  B38400 },
  { 57600,  B57600 },
  { 115200, B115200 },
  { 230400, B230400 },
  { 0,      0 }                 /* Terminator. */
};

int getibaud(const struct termios *termios_p) {
    return serial_baud_to_int(cfgetispeed(termios_p));
}

int getobaud(const struct termios *termios_p) {
    return serial_baud_to_int(cfgetospeed(termios_p));
}

speed_t serial_int_to_baud(int baud) {
    struct baud_mapping *map = baud_lookup_table;

    while (map->baud) {
      if (map->baud == baud)
        return map->speed;
      map++;
    }

    return baud;
}

int serial_baud_to_int(speed_t speed) {
    struct baud_mapping *map = baud_lookup_table;

    while (map->speed) {
      if (map->speed == speed)
        return map->baud;
      map++;
    }

    return speed;
}

void debug_termios(const char * msg, const struct termios *termios_p) {
    int ibaud, obaud, parity, stop;

    //Give debugging info
    ibaud = getibaud(termios_p);
    obaud = getobaud(termios_p);
    parity = CHECK_FLAG(termios_p->c_cflag, PARENB);
    stop = CHECK_FLAG(termios_p->c_cflag, CSTOPB)?2:1;
    printf("termios(%s) ibaud=%d, obaud=%d, parity=%d, stopbit=%d\n", msg, ibaud, obaud, parity, stop);
}


