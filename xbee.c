#include "xbee.h"

#include <stdlib.h>
#include <string.h>

#define HIGH_BYTE(x) ((uint8_t) (x) >> 8)
#define LOW_BYTE(x) ((uint8_t) (x) & 0x00FF)
#define COMB_BYTE(h,l) ((uint16_t) ((h << 8) + (l)))

#define BUFSIZE 1024

#define BUFOP_ADD 0
#define BUFOP_REM 1
#define BUFOP_GET 2
#define BUFOP_NOOP 3

#define XBEE_FRAME_START 0x7e

#define XBEE_LOC_START 0
#define XBEE_LOC_LEN_H 1
#define XBEE_LOC_LEN_L 2
#define XBEE_LOC_DATA  3
#define XBEE_LOC_CHECKSUM(l) (XBEE_LOC_DATA + (l) + 1)


int bufferop(int op, int datalen, uint8_t *data, uint8_t **buf) {
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
        case BUFOP_GET:
            *buf = buffer;
            return bufsize;
        break;
        case BUFOP_NOOP:
            return bufsize;
        break;
    }
}

uint8_t* getbuf() {
    uint8_t *buf;
    bufferop(BUFOP_GET, 0, NULL, &buf);
    return buf;
}

int addtobuf(int datalen, uint8_t *data) {
    return bufferop(BUFOP_ADD, datalen, data, NULL);
}

int remfrombuf(int datalen) {
    return bufferop(BUFOP_REM, datalen, NULL, NULL);
}

int sizebuf() {
    return bufferop(BUFOP_NOOP, 0, NULL, NULL);
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

    return EXIT_SUCCESS;
}

int getpacket(int fd, uint8_t *data, uint16_t maxlen) {
    // 1. Get more data from the serial and add to buffer
    // 2. Check if we have a valid packet yet
    //   a) Check the buffer has more than 5 bytes
    //   b) Check if the first byte is XBEE_FRAME_START, if not discard 1 byte
    //   c) Get the length
    //   d) Check the checksum value is within range
    //   e) Check the checksum is correct
    //   f) extract the data from the packet, store in data
    //   g) remove the packet from the buffer
}



