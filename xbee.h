#include <stdint.h>

void free_xbee_packet(uint8_t *packet);
uint8_t *make_xbee_packet(uint16_t length, uint8_t *data);


