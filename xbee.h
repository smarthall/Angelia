#include <stdint.h>

// Raw Packet Building
uint8_t *make_xbee_packet(uint16_t length);
void xbee_calc_checksum(uint8_t *packet);
void free_xbee_packet(uint8_t *packet);

// Predefined packet types
uint8_t *xbee_at_packet(const char *at_command);
uint8_t *xbee_at_packet_param(const char *at_command, uint16_t paramlen, const uint8_t *param);

