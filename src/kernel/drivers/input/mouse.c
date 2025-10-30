#include <aura/mouse.h>
#include <aura/interrupts.h>
#include <aura/pic.h>
#include <aura/io.h>
#include <aura/usb.h>
#include <stdint.h>

#define PS2_CMD 0x64
#define PS2_DATA 0x60

static volatile mouse_packet_t packet_buffer[16];
static volatile int packet_head = 0;
static volatile int packet_tail = 0;
static uint8_t packet_bytes[3];
static int packet_index = 0;

static void enqueue_packet(mouse_packet_t packet) {
    int next = (packet_head + 1) % (int) (sizeof(packet_buffer) / sizeof(packet_buffer[0]));
    if (next != packet_tail) {
        packet_buffer[packet_head] = packet;
        packet_head = next;
    }
}

static void mouse_write(uint8_t value) {
    while (inb(PS2_CMD) & 0x02) {
    }
    outb(PS2_CMD, 0xD4);
    while (inb(PS2_CMD) & 0x02) {
    }
    outb(PS2_DATA, value);
}

static uint8_t mouse_read(void) {
    while (!(inb(PS2_CMD) & 0x01)) {
    }
    return inb(PS2_DATA);
}

static void mouse_handler(interrupt_frame_t *frame) {
    (void)frame;
    uint8_t data = inb(PS2_DATA);
    packet_bytes[packet_index++] = data;
    if (packet_index == 3) {
        packet_index = 0;
        mouse_packet_t packet;
        packet.buttons = packet_bytes[0] & 0x07;
        packet.dx = (int8_t)packet_bytes[1];
        packet.dy = (int8_t)packet_bytes[2];
        enqueue_packet(packet);
    }
    pic_send_eoi(12);
}

void mouse_init(void) {
    register_interrupt_handler(44, mouse_handler);

    while (inb(PS2_CMD) & 0x02) {
    }
    outb(PS2_CMD, 0xA8);

    while (inb(PS2_CMD) & 0x02) {
    }
    outb(PS2_CMD, 0x20);
    uint8_t status = inb(PS2_DATA);
    status |= 0x02;
    while (inb(PS2_CMD) & 0x02) {
    }
    outb(PS2_CMD, 0x60);
    while (inb(PS2_CMD) & 0x02) {
    }
    outb(PS2_DATA, status);

    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();
}

int mouse_read_packet(mouse_packet_t *packet) {
    if (packet_tail == packet_head) {
        if (usb_poll_mouse(packet)) {
            return 1;
        }
        return 0;
    }
    *packet = packet_buffer[packet_tail];
    packet_tail = (packet_tail + 1) % (int) (sizeof(packet_buffer) / sizeof(packet_buffer[0]));
    return 1;
}
