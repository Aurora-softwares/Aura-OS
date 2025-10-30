#include <aura/keyboard.h>
#include <aura/interrupts.h>
#include <aura/pic.h>
#include <aura/io.h>
#include <aura/usb.h>
#include <aura/string.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64

static volatile char key_buffer[64];
static volatile int head = 0;
static volatile int tail = 0;
static int shift_pressed = 0;
static int caps_lock = 0;

static const char keymap[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`', 0,'\\','z','x','c','v','b','n','m',',','.','/', 0,'*', 0,' ',
};

static const char keymap_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','\"','~', 0,'|','Z','X','C','V','B','N','M','<','>','?', 0,'*', 0,' ',
};

static void enqueue_char(char c) {
    int next = (head + 1) % (int)sizeof(key_buffer);
    if (next != tail) {
        key_buffer[head] = c;
        head = next;
    }
}

static void ps2_keyboard_handler(interrupt_frame_t *frame) {
    (void)frame;
    uint8_t scancode = inb(PS2_DATA);
    if (scancode == 0xE0 || scancode == 0xE1) {
        pic_send_eoi(1);
        return;
    }

    if (scancode & 0x80) {
        uint8_t code = scancode & 0x7F;
        if (code == 0x2A || code == 0x36) {
            shift_pressed = 0;
        } else if (code == 0x3A) {
            caps_lock ^= 1;
        }
    } else {
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
        } else if (scancode == 0x3A) {
            caps_lock ^= 1;
        } else {
            char c = shift_pressed ? keymap_shift[scancode] : keymap[scancode];
            if (caps_lock && c >= 'a' && c <= 'z') {
                c -= 32;
            }
            if (c) {
                enqueue_char(c);
            }
        }
    }
    pic_send_eoi(1);
}

void keyboard_init(void) {
    register_interrupt_handler(33, ps2_keyboard_handler);
}

int keyboard_read_char(char *out) {
    if (tail == head) {
        if (usb_poll_keyboard(out)) {
            return 1;
        }
        return 0;
    }
    *out = key_buffer[tail];
    tail = (tail + 1) % (int)sizeof(key_buffer);
    return 1;
}
