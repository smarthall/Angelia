#include <stdint.h>

#define XBEE_PADDING 4

#define xbee_packet_size(packet) ((uint16_t) ((packet[1] << 8) + (packet[2]) + XBEE_PADDING))

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

// Packet Options
#define XBEE_DISABLE_ACK    0x01
#define XBEE_ENABLE_APS     0x20
#define XBEE_EXTEND_TIMEOUT 0x40

// Raw Packet Building
uint8_t *make_xbee_packet(uint16_t length);
void xbee_calc_checksum(uint8_t *packet);
void free_xbee_packet(uint8_t *packet);
int valid_xbee_packet(const uint8_t *packet);

// Predefined packet types
uint8_t *xbee_at_packet(const char *at_command);
uint8_t *xbee_at_packet_param(const char *at_command, uint16_t paramlen, const uint8_t *param);
uint8_t *xbee_rat_packet(const char *at_command, uint8_t *dest64);
uint8_t *xbee_rat_packet_param(const char *at_command, uint8_t *dest64, uint16_t paramlen, const uint8_t *param);
uint8_t *xbee_tx_packet(uint8_t *dest64, uint8_t options, uint16_t datalen, const uint8_t *data);

// Packet inspection
uint8_t xbee_frame_type(const uint8_t *packet);

// Debug
void print_xbee_packet(const char *msg, const uint8_t *packet);

