#include "mouse.h"

#include <stdint.h>

#include "assembly.h"
#include "screen.h"

#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64
#define PS2_DATA_PORT 0x60

#define PS2_STATUS_OUTPUT 0x01
#define PS2_STATUS_INPUT 0x02
#define PS2_STATUS_MOUSE_DATA 0x20

#define POINTER_CHAR 0xDB
#define POINTER_ATTR 0x0F

static uint8_t mouse_cycle;
static int8_t mouse_packet[3];
static uint8_t pointer_x = SCREEN_WIDTH / 2;
static uint8_t pointer_y = SCREEN_HEIGHT / 2;
static uint16_t pointer_background;
static uint8_t pointer_drawn;

static void mouse_wait(uint8_t type) {
    for (uint32_t timeout = 0; timeout < 100000; ++timeout) {
        uint8_t status = inb(PS2_STATUS_PORT);
        if (type == 0) {
            if (status & PS2_STATUS_OUTPUT) {
                return;
            }
        } else {
            if ((status & PS2_STATUS_INPUT) == 0) {
                return;
            }
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(PS2_COMMAND_PORT, 0xD4);
    mouse_wait(1);
    outb(PS2_DATA_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(PS2_DATA_PORT);
}

static void mouse_remove_pointer(void) {
    if (!pointer_drawn) {
        return;
    }

    screen_set_cell(pointer_x, pointer_y, pointer_background);
    pointer_drawn = 0;
}

static void mouse_draw_pointer(void) {
    uint16_t current_cell = screen_get_cell(pointer_x, pointer_y);
    if (!pointer_drawn || current_cell != ((uint16_t)POINTER_ATTR << 8 | POINTER_CHAR)) {
        pointer_background = current_cell;
    }

    screen_set_cell(pointer_x, pointer_y, ((uint16_t)POINTER_ATTR << 8) | POINTER_CHAR);
    pointer_drawn = 1;
}

void mouse_refresh_pointer(void) {
    mouse_draw_pointer();
}

static void mouse_apply_movement(int dx, int dy) {
    if (dx == 0 && dy == 0) {
        return;
    }

    mouse_remove_pointer();

    int new_x = pointer_x + dx;
    int new_y = pointer_y + dy;

    if (new_x < 0) {
        new_x = 0;
    } else if (new_x >= SCREEN_WIDTH) {
        new_x = SCREEN_WIDTH - 1;
    }

    if (new_y < 0) {
        new_y = 0;
    } else if (new_y >= SCREEN_HEIGHT) {
        new_y = SCREEN_HEIGHT - 1;
    }

    pointer_x = (uint8_t)new_x;
    pointer_y = (uint8_t)new_y;

    mouse_draw_pointer();
}

void mouse_init(void) {
    mouse_cycle = 0;
    pointer_drawn = 0;

    mouse_wait(1);
    outb(PS2_COMMAND_PORT, 0xA8); // Enable auxiliary device

    mouse_wait(1);
    outb(PS2_COMMAND_PORT, 0x20); // Read command byte
    mouse_wait(0);
    uint8_t status = inb(PS2_DATA_PORT);
    status |= 0x02;  // Enable IRQ12
    status |= 0x01;  // Enable IRQ1 just in case
    status &= ~0x20; // Use default clock speed

    mouse_wait(1);
    outb(PS2_COMMAND_PORT, 0x60);
    mouse_wait(1);
    outb(PS2_DATA_PORT, status);

    mouse_write(0xF6); // Set defaults
    mouse_read();
    mouse_write(0xF4); // Enable packet streaming
    mouse_read();

    mouse_draw_pointer();
}

void mouse_poll(void) {
    while (1) {
        uint8_t status = inb(PS2_STATUS_PORT);
        if (!(status & PS2_STATUS_OUTPUT)) {
            break;
        }

        if (!(status & PS2_STATUS_MOUSE_DATA)) {
            break;
        }

        int8_t data = (int8_t)inb(PS2_DATA_PORT);
        mouse_packet[mouse_cycle++] = data;

        if (mouse_cycle == 3) {
            mouse_cycle = 0;

            if (!(mouse_packet[0] & 0x08)) {
                continue;
            }

            int dx = mouse_packet[1];
            int dy = mouse_packet[2];

            if (mouse_packet[0] & 0x10) {
                dx |= 0xFFFFFF00;
            }

            if (mouse_packet[0] & 0x20) {
                dy |= 0xFFFFFF00;
            }

            mouse_apply_movement(dx, -dy);
        }
    }

    mouse_draw_pointer();
}
