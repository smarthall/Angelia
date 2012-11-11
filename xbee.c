#include "xbee.h"

#include <stdlib.h>
#include <string.h>

#define HIGH_BYTE(x) ((uint8_t) (x) >> 8)
#define LOW_BYTE(x) ((uint8_t) (x) & 0x00FF)
#define COMB_BYTE(h,l) ((uint16_t) ((h << 8) + (l)))

#define BUFSIZE 1024

#define BUFOP_ADD 0
#define BUFOP_REM 1

int bufferop(int op, int datalen, uint8_t *data) {
    static uint8_t buffer[BUFSIZE];
    static uint8_t bufsize = 0;

    switch (op) {
        case BUFOP_ADD:
            memcpy(buffer + bufsize, data, datalen);
            bufsize += datalen;
            return bufsize;
        break;
        case BUFOP_REM:
            bufsize -= datalen;
            memmove(buffer, buffer + datalen, bufsize);
            return bufsize;
        break;
    }
}

int addtobuf(int datalen, uint8_t *data) {
    return bufferop(BUFOP_ADD, datalen, data);
}

int remfrombuf(int datalen) {
    return bufferop(BUFOP_REM, datalen, NULL);
}

int sendpacket(int fd, uint16_t len, uint8_t *data) {
    uint8_t checksum, start, len_h, len_l;
    int i;

    start = 0x7e;
    len_h = HIGH_BYTE(len);
    len_l = LOW_BYTE(len);

    for (i = 0; i < len; i++) {
        checksum += data[i];
    }

    checksum = 0xFF - checksum;

    write(fd, &start, 1);
    write(fd, &len_h, 1);
    write(fd, &len_l, 1);
    write(fd, data, len);
    write(fd, &checksum, 1);
}

int getpacket(int fd, uint8_t *data, uint16_t maxlen) {
}



