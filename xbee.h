#include <stdint.h>

#define xbee_packet_size(packet) ((uint16_t) ((packet[1] << 8) + (packet[2])))

// Packet Options
#define XBEE_DISABLE_ACK    0x01
#define XBEE_ENABLE_APS     0x20
#define XBEE_EXTEND_TIMEOUT 0x40

// Raw Packet Building
uint8_t *make_xbee_packet(uint16_t length);
void xbee_calc_checksum(uint8_t *packet);
void free_xbee_packet(uint8_t *packet);

// Predefined packet types
uint8_t *xbee_at_packet(const char *at_command);
uint8_t *xbee_at_packet_param(const char *at_command, uint16_t paramlen, const uint8_t *param);
uint8_t *xbee_rat_packet(const char *at_command, uint8_t *dest64);
uint8_t *xbee_rat_packet_param(const char *at_command, uint8_t *dest64, uint16_t paramlen, const uint8_t *param);
uint8_t *xbee_tx_packet(const char *at_command, uint8_t *dest64, uint8_t options, uint16_t datalen, const uint8_t *data);

