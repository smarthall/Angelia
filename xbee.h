#include <stdint.h>

uint8_t *make_xbee_packet(uint16_t length);
void xbee_calc_checksum(uint8_t *packet);
void free_xbee_packet(uint8_t *packet);


