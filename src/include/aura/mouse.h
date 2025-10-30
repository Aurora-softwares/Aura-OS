#ifndef AURA_MOUSE_H
#define AURA_MOUSE_H

#include <stdint.h>

typedef struct {
    int dx;
    int dy;
    uint8_t buttons;
} mouse_packet_t;

void mouse_init(void);
int mouse_read_packet(mouse_packet_t *packet);

#endif
