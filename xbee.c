#include <stdlib.h>
#include <string.h>
#include "xbee.h"

// Macros to make byte playing easier
#define HIGH_BYTE(x)    ((uint8_t) (x) >> 8)
#define LOW_BYTE(x)     ((uint8_t) (x) & 0x00FF)
#define COMB_BYTE(h,l)  ((uint16_t) ((h << 8) + (l)))

// Other defines
#define XBEE_PADDING 4

// API Value Codes
#define XBEE_START    0x7E
#define XBEE_CMD_AT   0x08
#define XBEE_CMD_ATQ  0x09
#define XBEE_CMD_TX   0x10
#define XBEE_CMD_TXC  0x11
#define XBEE_CMD_RAT  0x17
#define XBEE_CMD_CSR  0x21
#define XBEE_CMD_ATR  0x88
#define XBEE_CMD_MS   0x8A
#define XBEE_CMD_TS   0x8B
#define XBEE_CMD_RP   0x90
#define XBEE_CMD_RXI  0x91
#define XBEE_CMD_SRXI 0x92
#define XBEE_CMD_SRI  0x94
#define XBEE_CMD_NII  0x95
#define XBEE_CMD_RATR 0x97
#define XBEE_CMD_FWU  0xA0
#define XBEE_CMD_RII  0xA1
#define XBEE_CMD_MRRI 0xA3

// Packet Position Codes
#define LOC_START    0x00
#define LOC_LENGTH_H 0x01
#define LOC_LENGTH_L 0x02
#define LOC_DATA     0x03



uint8_t *make_xbee_packet(uint16_t length) {
    uint8_t *packet = malloc((length + XBEE_PADDING) * sizeof(uint8_t));
    uint8_t checksum;
    int i;

    packet[LOC_START] = XBEE_START;
    packet[LOC_LENGTH_H] = HIGH_BYTE(length);
    packet[LOC_LENGTH_L] = LOW_BYTE(length);

    return packet;
}

void xbee_calc_checksum(uint8_t *packet) {
    uint8_t *current, *end, checksum;
    uint16_t length = COMB_BYTE(packet[LOC_LENGTH_H], packet[LOC_LENGTH_L]);

    current = packet + LOC_DATA;
    end = packet + LOC_DATA + length;

    while (current < end) {
        checksum += *current;
        current++;
    }

    packet[LOC_DATA + length] = 0xFF - checksum;

}

void free_xbee_packet(uint8_t *packet) {
    free (packet);
}

uint8_t *xbee_at_packet(const char *at_command) {
    return xbee_at_packet_param(at_command, 0, NULL);
}

uint8_t *xbee_at_packet_param(const char *at_command, uint16_t paramlen, const uint8_t *param) {
    uint16_t payloadlen = 2 + paramlen;
    uint8_t *packet;

    packet = make_xbee_packet(payloadlen);
    packet[LOC_DATA] = XBEE_CMD_AT;
    packet[LOC_DATA + 1] = at_command[0];
    packet[LOC_DATA + 2] = at_command[1];
    if (paramlen > 0) memcpy(packet + LOC_DATA + 3, param, paramlen);

    xbee_calc_checksum(packet);

    return packet;
}

