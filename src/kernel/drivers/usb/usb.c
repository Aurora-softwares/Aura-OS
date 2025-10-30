#include <aura/usb.h>
#include <aura/pci.h>
#include <aura/io.h>
#include <aura/string.h>
#include <aura/log.h>
#include <stdint.h>

#define MAX_USB_DEVICES 8

#define PCI_CLASS_SERIAL 0x0C
#define PCI_SUBCLASS_USB 0x03
#define PCI_PROGIF_UHCI  0x00

#define UHCI_REG_USBCMD      0x00
#define UHCI_REG_USBSTS      0x02
#define UHCI_REG_FRNUM       0x06
#define UHCI_REG_FRBASEADD   0x08
#define UHCI_REG_PORTSC1     0x10
#define UHCI_REG_PORTSC2     0x12

#define TD_CTRL_ACTIVE   (1 << 23)
#define TD_CTRL_STALLED  (1 << 22)
#define TD_CTRL_BABBLE   (1 << 20)
#define TD_CTRL_NAK      (1 << 19)
#define TD_CTRL_TIMEOUT  (1 << 18)
#define TD_CTRL_BITSTUFF (1 << 17)
#define TD_CTRL_C_ERR_SHIFT 27
#define TD_CTRL_IOC      (1 << 24)
#define TD_CTRL_SPD      (1 << 29)

#define TD_PID_SETUP 0x2D
#define TD_PID_IN    0x69
#define TD_PID_OUT   0xE1

struct uhci_td {
    uint32_t link_ptr;
    uint32_t control;
    uint32_t token;
    uint32_t buffer;
} __attribute__((aligned(16)));

struct uhci_qh {
    uint32_t head_link;
    uint32_t element_link;
} __attribute__((aligned(16)));

typedef struct {
    uint16_t io_base;
    int present;
    uint32_t frame_list[1024] __attribute__((aligned(4096)));
    struct uhci_qh control_qh;
    struct uhci_td control_tds[3];
} uhci_controller_t;

static uhci_controller_t uhci;
static usb_device_t device_list[MAX_USB_DEVICES];
static int device_count = 0;

static inline uint16_t uhci_read16(uint16_t base, uint16_t reg) {
    return inw(base + reg);
}

static inline void uhci_write16(uint16_t base, uint16_t reg, uint16_t value) {
    outw(base + reg, value);
}

static inline void uhci_wait(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; ++i) {
        __asm__ volatile ("nop");
    }
}

static void uhci_reset(uhci_controller_t *ctl) {
    uint16_t base = ctl->io_base;
    uhci_write16(base, UHCI_REG_USBCMD, 0x0002);
    while (uhci_read16(base, UHCI_REG_USBCMD) & 0x0002) {
    }
    uhci_write16(base, UHCI_REG_USBSTS, 0xFFFF);
}

static void uhci_start(uhci_controller_t *ctl) {
    uint16_t base = ctl->io_base;
    uhci_write16(base, UHCI_REG_FRNUM, 0);
    uhci_write16(base, UHCI_REG_FRBASEADD, (uint16_t)((uintptr_t)ctl->frame_list));
    uhci_write16(base, UHCI_REG_USBCMD, 0x0001 | 0x0008);
}

static int uhci_control_transfer(uhci_controller_t *ctl, uint8_t address, const void *setup_data, size_t setup_length, void *data, size_t length, uint8_t data_pid) {
    memset(&ctl->control_qh, 0, sizeof(ctl->control_qh));
    memset(ctl->control_tds, 0, sizeof(ctl->control_tds));

    struct uhci_td *setup_td = &ctl->control_tds[0];
    struct uhci_td *data_td = &ctl->control_tds[1];
    struct uhci_td *status_td = &ctl->control_tds[2];

    setup_td->link_ptr = (uint32_t)(uintptr_t)data_td;
    setup_td->control = TD_CTRL_ACTIVE | (3 << TD_CTRL_C_ERR_SHIFT);
    setup_td->token = (TD_PID_SETUP) | ((uint32_t)address << 8) | (0 << 15) | (0 << 19) | ((setup_length ? setup_length - 1 : 0x7FF) << 21);
    setup_td->buffer = (uint32_t)(uintptr_t)setup_data;

    if (length > 0) {
        data_td->link_ptr = (uint32_t)(uintptr_t)status_td;
        data_td->control = TD_CTRL_ACTIVE | (3 << TD_CTRL_C_ERR_SHIFT) | TD_CTRL_IOC;
        data_td->token = ((uint32_t)data_pid) | ((uint32_t)address << 8) | (0 << 15) | (1 << 19) | ((length ? length - 1 : 0x7FF) << 21);
        data_td->buffer = (uint32_t)(uintptr_t)data;
    } else {
        data_td->link_ptr = (uint32_t)(uintptr_t)status_td;
        data_td->control = TD_CTRL_ACTIVE | (3 << TD_CTRL_C_ERR_SHIFT) | TD_CTRL_IOC;
        data_td->token = ((uint32_t)TD_PID_IN) | ((uint32_t)address << 8) | (0 << 15) | (1 << 19) | (0x7FF << 21);
        data_td->buffer = 0;
    }

    status_td->link_ptr = 0x00000001;
    status_td->control = TD_CTRL_ACTIVE | (3 << TD_CTRL_C_ERR_SHIFT) | TD_CTRL_IOC | TD_CTRL_SPD;
    status_td->token = ((uint32_t)((length > 0) ? ((data_pid == TD_PID_IN) ? TD_PID_OUT : TD_PID_IN) : TD_PID_IN)) |
                       ((uint32_t)address << 8) | (0 << 15) | (1 << 19) | (0x7FF << 21);
    status_td->buffer = 0;

    ctl->control_qh.head_link = 0x00000001;
    ctl->control_qh.element_link = (uint32_t)(uintptr_t)setup_td;

    for (int i = 0; i < 1024; ++i) {
        ctl->frame_list[i] = (uint32_t)(uintptr_t)&ctl->control_qh | 0x00000002;
    }

    uhci_start(ctl);

    uint16_t base = ctl->io_base;
    uint32_t timeout = 1000000;
    while ((status_td->control & TD_CTRL_ACTIVE) && timeout--) {
        (void)uhci_read16(base, UHCI_REG_USBSTS);
    }

    if (status_td->control & TD_CTRL_ACTIVE) {
        return -1;
    }

    if (status_td->control & (TD_CTRL_STALLED | TD_CTRL_BABBLE | TD_CTRL_NAK | TD_CTRL_TIMEOUT | TD_CTRL_BITSTUFF)) {
        return -1;
    }

    return 0;
}

struct usb_setup_packet {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed));

static int usb_request(uhci_controller_t *ctl, uint8_t address, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, void *buffer, uint16_t length) {
    struct usb_setup_packet setup = {
        .bmRequestType = request_type,
        .bRequest = request,
        .wValue = value,
        .wIndex = index,
        .wLength = length,
    };
    uint8_t data_pid = (request_type & 0x80) ? TD_PID_IN : TD_PID_OUT;
    return uhci_control_transfer(ctl, address, &setup, sizeof(setup), buffer, length, data_pid);
}

static void usb_reset_port(uhci_controller_t *ctl, uint16_t port_reg) {
    uint16_t base = ctl->io_base;
    uint16_t value = uhci_read16(base, port_reg);
    uhci_write16(base, port_reg, value | (1 << 9));
    uhci_wait(1000);
    uhci_write16(base, port_reg, value & ~(1 << 9));
    uhci_wait(1000);
}

static void enumerate_port(uhci_controller_t *ctl, uint16_t port_reg) {
    uint16_t base = ctl->io_base;
    uint16_t status = uhci_read16(base, port_reg);
    if (!(status & 0x0001)) {
        return;
    }

    usb_reset_port(ctl, port_reg);
    status = uhci_read16(base, port_reg);
    if (!(status & 0x0004)) {
        return;
    }

    uhci_write16(base, port_reg, status | 0x0004);

    uint8_t address = (uint8_t)(device_count + 1);
    uint8_t buffer[64];
    if (usb_request(ctl, 0, 0x80, 6, 0x0100, 0, buffer, 8) != 0) {
        return;
    }

    if (usb_request(ctl, 0, 0x00, 5, address, 0, NULL, 0) != 0) {
        return;
    }
    uhci_wait(10000);

    if (usb_request(ctl, address, 0x80, 6, 0x0200, 0, buffer, sizeof(buffer)) != 0) {
        return;
    }

    uint8_t class = buffer[5];
    uint8_t subclass = buffer[6];
    uint8_t protocol = buffer[7];
    usb_device_type_t type = USB_DEVICE_NONE;
    if (class == 3 && subclass == 1) {
        if (protocol == 1) {
            type = USB_DEVICE_KEYBOARD;
        } else if (protocol == 2) {
            type = USB_DEVICE_MOUSE;
        }
    }

    if (usb_request(ctl, address, 0x21, 0x0A, 0, 0, NULL, 0) != 0) {
        return;
    }

    if (usb_request(ctl, address, 0x21, 0x0B, 0, 0, NULL, 0) != 0) {
        return;
    }

    if (type != USB_DEVICE_NONE && device_count < MAX_USB_DEVICES) {
        usb_device_t *dev = &device_list[device_count++];
        dev->type = type;
        dev->address = address;
        dev->interface_number = 0;
        dev->endpoint_in = 1;
        dev->endpoint_out = 0;
    }
}

void usb_init(void) {
    pci_device_t devices[32];
    int count = pci_enumerate(devices, 32);
    for (int i = 0; i < count; ++i) {
        if (devices[i].class_code == PCI_CLASS_SERIAL &&
            devices[i].subclass == PCI_SUBCLASS_USB &&
            devices[i].prog_if == PCI_PROGIF_UHCI) {
            uint32_t bar4 = pci_read_config32(devices[i].bus, devices[i].device, devices[i].function, 0x20);
            uhci.io_base = (uint16_t)(bar4 & 0xFFF0);
            uhci.present = 1;
            break;
        }
    }

    if (!uhci.present) {
        log_info("UHCI controller not found; USB support limited");
        return;
    }

    uhci_reset(&uhci);
    memset(uhci.frame_list, 0, sizeof(uhci.frame_list));
    enumerate_port(&uhci, UHCI_REG_PORTSC1);
    enumerate_port(&uhci, UHCI_REG_PORTSC2);
}

const usb_device_t *usb_find_keyboard(void) {
    for (int i = 0; i < device_count; ++i) {
        if (device_list[i].type == USB_DEVICE_KEYBOARD) {
            return &device_list[i];
        }
    }
    return NULL;
}

const usb_device_t *usb_find_mouse(void) {
    for (int i = 0; i < device_count; ++i) {
        if (device_list[i].type == USB_DEVICE_MOUSE) {
            return &device_list[i];
        }
    }
    return NULL;
}

int usb_poll_keyboard(char *ch) {
    const usb_device_t *dev = usb_find_keyboard();
    if (!dev || !uhci.present) {
        return 0;
    }

    uint8_t data[8];
    if (usb_request(&uhci, dev->address, 0xA1, 0x01, 0x0100, dev->interface_number, data, sizeof(data)) != 0) {
        return 0;
    }

    for (int i = 2; i < 8; ++i) {
        if (data[i]) {
            *ch = (char)data[i];
            return 1;
        }
    }
    return 0;
}

int usb_poll_mouse(mouse_packet_t *packet) {
    const usb_device_t *dev = usb_find_mouse();
    if (!dev || !uhci.present) {
        return 0;
    }

    uint8_t data[4];
    if (usb_request(&uhci, dev->address, 0xA1, 0x01, 0x0100, dev->interface_number, data, sizeof(data)) != 0) {
        return 0;
    }

    packet->buttons = data[0] & 0x07;
    packet->dx = (int8_t)data[1];
    packet->dy = (int8_t)data[2];
    return 1;
}
