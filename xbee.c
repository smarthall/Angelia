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

void print_hex_data(const uint8_t *data, int len) {
    int i;

    for (i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }

}

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
    int i, length = COMB_BYTE(packet[LOC_LENGTH_H], packet[LOC_LENGTH_L]);

    printf("-------PACKET-------\n");
    printf("Comment: %s\n", msg);
    printf("Raw Data: 0x"); print_hex_data(packet, length + XBEE_PADDING); printf("\n");
    printf("Length: %d\n", length);

    switch (packet[LOC_DATA]) {
        case XBEE_CMD_AT:
          printf("Packet Type: 0x08 - AT Command\n");
          printf("Frame ID: 0x%02x\n", packet[LOC_DATA + 1]);
          printf("AT Command: '%c%c'\n", packet[LOC_DATA + 2], packet[LOC_DATA + 3]);
          printf("Params: "); print_hex_data(packet + LOC_DATA + 3, length - 4); printf("\n");
          break;
        case 0x17:
          printf("Packet Type: 0x17 - Remote AT Command Request\n");
          break;
        case 0x88:
          printf("Packet Type: 0x88 - AT Command Response\n");
          printf("Frame ID: 0x%02x\n", packet[LOC_DATA + 1]);
          printf("AT Command: '%c%c'\n", packet[LOC_DATA + 2], packet[LOC_DATA + 3]);
          switch (packet[LOC_DATA + 4]) {
              case 0x00:
                printf("Command Status: OK\n");
              break;
              case 0x01:
                printf("Command Status: ERROR\n");
              break;
              case 0x02:
                printf("Command Status: Invalid Command\n");
              break;
              case 0x03:
                printf("Command Status: Invalid Parameter\n");
              break;
              case 0x04:
                printf("Command Status: Tx Failure\n");
              break;
          }
          printf("Data: "); print_hex_data(packet + LOC_DATA + 5, length - 5); printf("\n");
          break;
        case 0x97:
          printf("Packet Type: 0x97 - Remote AT Command Response\n");
          break;
        default:
          printf("Packet Type: 0x%02x - Unknown\n", packet[LOC_DATA]);
    }

    printf("-----END PACKET-----\n");
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
    uint16_t payloadlen = 15 + paramlen;
    uint8_t *packet;

    packet = make_xbee_packet(payloadlen);
    packet[LOC_DATA] = XBEE_CMD_RAT;
    packet[LOC_DATA + 1] = 24;
    memcpy(packet + LOC_DATA + 2, dest64, sizeof(uint8_t) * 8);
    packet[LOC_DATA + 10] = 0xFF;
    packet[LOC_DATA + 11] = 0xFE;
    packet[LOC_DATA + 12] = 0x02;
    packet[LOC_DATA + 13] = at_command[0];
    packet[LOC_DATA + 14] = at_command[1];
    if (paramlen > 0) memcpy(packet + LOC_DATA + 15, param, sizeof(uint8_t) * paramlen);

    xbee_calc_checksum(packet);

    return packet;
}

uint8_t *xbee_tx_packet(uint8_t *dest64, uint8_t options, uint16_t datalen, const uint8_t *data) {
    uint16_t payloadlen = 14 + datalen;
    uint8_t *packet;

    packet = make_xbee_packet(payloadlen);
    packet[LOC_DATA] = XBEE_CMD_TX;
    packet[LOC_DATA + 1] = 0;
    memcpy(packet + LOC_DATA + 2, dest64, sizeof(uint8_t) * 8);
    packet[LOC_DATA + 10] = 0xFF;
    packet[LOC_DATA + 11] = 0xFE;
    packet[LOC_DATA + 12] = 0x00;
    packet[LOC_DATA + 13] = options;
    if (datalen > 0) memcpy(packet + LOC_DATA + 14, data, datalen);

    xbee_calc_checksum(packet);

    return packet;
}


