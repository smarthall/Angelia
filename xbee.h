#include <stdint.h>

#define xbee_packet_size(packet) ((uint16_t) ((packet[1] << 8) + (packet[2])))

// Raw Packet Building
uint8_t *make_xbee_packet(uint16_t length);
void xbee_calc_checksum(uint8_t *packet);
void free_xbee_packet(uint8_t *packet);

// Predefined packet types
uint8_t *xbee_at_packet(const char *at_command);
uint8_t *xbee_at_packet_param(const char *at_command, uint16_t paramlen, const uint8_t *param);
uint8_t *xbee_rat_packet(const char *at_command, uint8_t *dest64);
uint8_t *xbee_rat_packet_param(const char *at_command, uint8_t *dest64, uint16_t paramlen, const uint8_t *param);

