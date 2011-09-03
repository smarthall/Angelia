#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xbee.h"

// Macros to make byte playing easier
#define HIGH_BYTE(x)    ((uint8_t) (x) >> 8)
#define LOW_BYTE(x)     ((uint8_t) (x) & 0x00FF)
#define COMB_BYTE(h,l)  ((uint16_t) ((h << 8) + (l)))

// Packet Position Codes
#define LOC_START    0x00
#define LOC_LENGTH_H 0x01
#define LOC_LENGTH_L 0x02
#define LOC_DATA     0x03

uint8_t *make_xbee_packet(uint16_t length) {
    uint8_t *packet = malloc(length + XBEE_PADDING);
    uint8_t checksum;
    int i;

    packet[LOC_START] = XBEE_START;
    packet[LOC_LENGTH_H] = HIGH_BYTE(length);
    packet[LOC_LENGTH_L] = LOW_BYTE(length);

    return packet;
}

void print_xbee_packet(const char *msg, const uint8_t *packet) {
    int i, length = COMB_BYTE(packet[LOC_LENGTH_H], packet[LOC_LENGTH_L] + XBEE_PADDING);
    printf("XBee Packet(%s): 0x", msg);
    for (i = 0; i < length; i++) {
        printf("%02x", packet[i]);
    }
    printf("\n");
}

void xbee_calc_checksum(uint8_t *packet) {
    uint8_t checksum = 0, i;
    uint16_t length = COMB_BYTE(packet[LOC_LENGTH_H], packet[LOC_LENGTH_L]);

    for (i = LOC_DATA; i < (LOC_DATA + length); i++) {
        checksum += packet[i];
    }

    packet[LOC_DATA + length] = 0xFF - checksum;

}

int valid_xbee_packet(const uint8_t *packet) {
    uint8_t checksum = 0, i;
    uint16_t length = COMB_BYTE(packet[LOC_LENGTH_H], packet[LOC_LENGTH_L]);

    for (i = LOC_DATA; i < (LOC_DATA + length); i++) {
        checksum += packet[i];
    }

    return (packet[LOC_DATA + length] == (0xFF - checksum));
}

uint8_t xbee_frame_type(const uint8_t *packet) {
    if (!valid_xbee_packet(packet)) return 0;

    return packet[LOC_DATA];
}

void free_xbee_packet(uint8_t *packet) {
    free (packet);
}

uint8_t *xbee_at_packet(const char *at_command) {
    return xbee_at_packet_param(at_command, 0, NULL);
}

uint8_t *xbee_at_packet_param(const char *at_command, uint16_t paramlen, const uint8_t *param) {
    uint16_t payloadlen = 4 + paramlen;
    uint8_t *packet;

    packet = make_xbee_packet(payloadlen);
    packet[LOC_DATA] = XBEE_CMD_AT;
    packet[LOC_DATA + 1] = 0x52;
    packet[LOC_DATA + 2] = at_command[0];
    packet[LOC_DATA + 3] = at_command[1];
    if (paramlen > 0) memcpy(packet + LOC_DATA + 4, param, paramlen);

    xbee_calc_checksum(packet);

    return packet;
}

uint8_t *xbee_rat_packet(const char *at_command, uint8_t *dest64) {
    return xbee_rat_packet_param(at_command, dest64, 0, NULL);
}

uint8_t *xbee_rat_packet_param(const char *at_command, uint8_t *dest64, uint16_t paramlen, const uint8_t *param) {
    uint16_t payloadlen = 10 + paramlen;
    uint8_t *packet;

    packet = make_xbee_packet(payloadlen);
    packet[LOC_DATA] = XBEE_CMD_RAT;
    packet[LOC_DATA + 1] = 0;
    memcpy(packet + LOC_DATA + 2, dest64, sizeof(uint8_t) * 8);
    packet[LOC_DATA + 3] = 0xFF;
    packet[LOC_DATA + 4] = 0xFE;
    packet[LOC_DATA + 5] = at_command[0];
    packet[LOC_DATA + 6] = at_command[1];
    if (paramlen > 0) memcpy(packet + LOC_DATA + 7, param, paramlen);

    xbee_calc_checksum(packet);

    return packet;
}

uint8_t *xbee_tx_packet(uint8_t *dest64, uint8_t options, uint16_t datalen, const uint8_t *data) {
    uint16_t payloadlen = 12 + datalen;
    uint8_t *packet;

    packet = make_xbee_packet(payloadlen);
    packet[LOC_DATA] = XBEE_CMD_TX;
    packet[LOC_DATA + 1] = 0;
    memcpy(packet + LOC_DATA + 2, dest64, sizeof(uint8_t) * 8);
    packet[LOC_DATA + 3] = 0xFF;
    packet[LOC_DATA + 4] = 0xFE;
    packet[LOC_DATA + 5] = 0x00;
    packet[LOC_DATA + 6] = options;
    if (datalen > 0) memcpy(packet + LOC_DATA + 7, data, datalen);

    xbee_calc_checksum(packet);

    return packet;
}


