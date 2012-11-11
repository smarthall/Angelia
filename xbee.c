#include "xbee.h"

#define HIGH_BYTE(x) ((uint8_t) (x) >> 8)
#define LOW_BYTE(x) ((uint8_t) (x) & 0x00FF)
#define COMB_BYTE(h,l) ((uint16_t) ((h << 8) + (l)))

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


